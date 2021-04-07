#include <algorithm>
#include "../../include/UbiForm/Component.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define SUBSCRIBER_COMPONENT "SUBSCRIBER"
#define PUBLISHER_COMPONENT "PUBLISHER"
#define BROKER_COMPONENT "BROKER"
#define MESSAGE_NUM 1000

struct subscriberStartupData{
    Component* component;
    bool done = false;
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
    int messagesLost = 0;
    int messagesReceived;
};

void subscriberStartup(Endpoint* e, void*d){
    auto* userData = static_cast<subscriberStartupData*>(d);
    DataReceiverEndpoint* subscriber = userData->component->castToDataReceiverEndpoint(e);

    auto msg = subscriber->receiveMessage();
    int localCounter = msg->getInteger("counter") + 1;
    int initial = localCounter;
    userData->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    for ( ; localCounter <MESSAGE_NUM; localCounter++){
        try {
            msg = subscriber->receiveMessage();
            if (msg->getInteger("counter") != localCounter) {
                userData->messagesLost ++;
                localCounter = msg->getInteger("counter");
            }
        }catch (std::logic_error &e){
            break;
        }
    }
    userData->messagesReceived = localCounter - userData->messagesLost - initial;
    userData->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    userData->done = true;
}


struct BrokerReqEndpointStartupData{
    Component* component;
    std::string publisherEndpointSchemaName;
};

void brokerRequestEndpointStartup(Endpoint* e, void* data){
    auto* userData = static_cast<BrokerReqEndpointStartupData*>(data);

    auto reqReceive = userData->component->castToDataReceiverEndpoint(e);
    auto reqSend = userData->component->castToDataSenderEndpoint(e);


    std::shared_ptr<DataSenderEndpoint> publisher;
    try{
        userData->component->createEndpointAndListen(userData->publisherEndpointSchemaName);
        auto publishers = userData->component->getEndpointsByType(userData->publisherEndpointSchemaName);
        if (publishers->empty()){
            throw std::logic_error("Couldn't find endpoint");
        }
        publisher = userData->component->castToDataSenderEndpoint(publishers->at(0));
    } catch (std::logic_error &e){
        std::cerr << "Unable to create publisher\n" << e.what() << std::endl;
        return;
    }

    while (true){
        std::unique_ptr<EndpointMessage> msg;
        try{
            msg = reqReceive->receiveMessage();
            EndpointMessage sm;
            sm.addMember("success",true);
            reqSend->sendMessage(sm);
        } catch (ValidationError &e) {
            EndpointMessage sm;
            sm.addMember("success", false);
            continue;
        } catch (std::logic_error &e){
            //EXITING
            break;
        }

        try{
            publisher->sendMessage(*msg);
        } catch (std::logic_error &e){
            //EXITING
            break;
        }
    }
    delete userData;
}



struct BrokerEndpointAdditionData{
    Component * component;
};

void brokerEndpointAddition(std::string endpointType, void* data){
    auto* userData = static_cast<BrokerEndpointAdditionData*>(data);
    if(userData->component->getComponentManifest().hasEndpoint(endpointType) &&
        userData->component->getComponentManifest().getConnectionParadigm(endpointType) == convertFromConnectionParadigm(ConnectionParadigm::Reply)){
        auto inputSchema = userData->component->getComponentManifest().getReceiverSchema(endpointType);
        userData->component->getComponentManifest().addEndpoint(ConnectionParadigm::Publisher,endpointType + "_publisher",
                                                                nullptr,inputSchema);

        auto* startupData = new BrokerReqEndpointStartupData;
        startupData->component = userData->component;
        startupData->publisherEndpointSchemaName = endpointType + "_publisher";
        userData->component->registerStartupFunction(endpointType,brokerRequestEndpointStartup, startupData);
    }

}


int main(int argc, char **argv) {
    Component component;

    component.startBackgroundListen();
    auto *data = new BrokerEndpointAdditionData;
    data->component = &component;
    component.getComponentManifest().registerEndpointAdditionCallback(brokerEndpointAddition, data);
    while (true) {
        nng_msleep(1000);
    }


}