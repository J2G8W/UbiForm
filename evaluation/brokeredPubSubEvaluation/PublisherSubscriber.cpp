#include <algorithm>
#include "../../include/UbiForm/Component.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define SUBSCRIBER_COMPONENT "SUBSCRIBER"
#define PUBLISHER_COMPONENT "PUBLISHER"
#define MESSAGE_NUM 1000

struct subscriberStartupData{
    Component* component;
    bool done = false;
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
    std::vector<std::chrono::duration<int64_t, std::nano>> messageReceives;
    int messagesLost = 0;
    int messagesReceived;
};

void subscriberStartup(Endpoint* e, void*d){
    auto* userData = static_cast<subscriberStartupData*>(d);
    DataReceiverEndpoint* subscriber = userData->component->castToDataReceiverEndpoint(e);

    auto msg = subscriber->receiveMessage();
    int localCounter = msg->getInteger("counter") + 1;
    int initial = localCounter;
    userData->messageReceives.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
    userData->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    for ( ; localCounter <MESSAGE_NUM; localCounter++){
        try {
            msg = subscriber->receiveMessage();
            if (msg->getInteger("counter") != localCounter) {
                userData->messagesLost ++;
                localCounter = msg->getInteger("counter");
                userData->messageReceives.push_back(std::chrono::duration<int64_t, std::nano>::zero());
            }else{
                userData->messageReceives.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            }
        }catch (std::logic_error &e){
            break;
        }
    }
    userData->messagesReceived = localCounter - userData->messagesLost - initial;
    userData->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    userData->done = true;
}

std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}


int main(int argc, char **argv) {
    if (strcmp(argv[1], SUBSCRIBER_COMPONENT) == 0 && argc >= 3) {
        Component component;

        std::shared_ptr <EndpointSchema> es = std::make_shared<EndpointSchema>();
        es->addProperty("counter", ValueType::Number);
        es->addRequired("counter");
        es->addProperty("extraData",ValueType::String);
        es->addRequired("extraData");
        component.getComponentManifest().addEndpoint(ConnectionParadigm::Subscriber, "speedSub", es, nullptr);

        auto *userData = new subscriberStartupData;
        userData->component = &component;
        userData->messageReceives.reserve(MESSAGE_NUM);
        component.registerStartupFunction("speedSub", subscriberStartup, userData);

        std::string brokerAddress = argv[2];
        component.getBackgroundRequester().requestRemoteListenThenDial(
                brokerAddress, 8000, "speedSub",
                "dataBroker_publisher");

        while (!userData->done) {
            nng_msleep(500);
        }
        userData->duration = userData->endTime - userData->startTime;

        std::ofstream results;
        results.open("broker_subscriber_results.txt", std::fstream::out | std::fstream::app);
        results << userData->duration.count() << "," << userData->messagesReceived << "," << userData->messagesLost
                << "\n";
        for(auto i : userData->messageReceives){
            results << i.count() << ",";
        }
        results << std::endl;
        results.close();
    } else if (strcmp(argv[1], PUBLISHER_COMPONENT) == 0) {
        Component component;

        std::shared_ptr <EndpointSchema> sendSchema = std::make_shared<EndpointSchema>();
        sendSchema->addProperty("counter", ValueType::Number);
        sendSchema->addRequired("counter");
        sendSchema->addProperty("extraData",ValueType::String);
        sendSchema->addRequired("extraData");

        std::shared_ptr <EndpointSchema> receiveSchema = std::make_shared<EndpointSchema>();
        receiveSchema->addProperty("success", ValueType::Boolean);
        receiveSchema->addRequired("success");
        component.getComponentManifest().addEndpoint(ConnectionParadigm::Request, "dataPublisher",
                                                     receiveSchema, sendSchema);

        component.startBackgroundListen();


        std::vector<std::string> randomStrings(MESSAGE_NUM);
        for(int i = 0; i < MESSAGE_NUM; i++){
            randomStrings[i] = random_string(300);
        }


        std::vector<std::chrono::duration<int64_t, std::nano>> messageSendTimes;
        std::vector<std::chrono::duration<int64_t, std::nano>> messageReplyTimes;
        messageSendTimes.reserve(MESSAGE_NUM);
        messageReplyTimes.reserve(MESSAGE_NUM);


        std::string brokerAddress = argv[2];
        component.getBackgroundRequester().requestChangeEndpoint(brokerAddress+":8000", ConnectionParadigm::Reply,
                                                                 "dataBroker", sendSchema.get(), receiveSchema.get());

        nng_msleep(2000);

        component.getBackgroundRequester().requestRemoteListenThenDial(brokerAddress,8000, "dataPublisher","dataBroker");

        auto endpoints = component.getEndpointsByType("dataPublisher");
        if (endpoints->empty()) {
            std::cerr << "Unable to make endpoint" << std::endl;
            return 1;
        }
        auto reqEndpoint = endpoints->at(0);
        auto sendEndpoint = component.castToDataSenderEndpoint(reqEndpoint);
        auto recvEndpoint = component.castToDataReceiverEndpoint(reqEndpoint);


        EndpointMessage sm;
        int i = 0;
        for (; i < MESSAGE_NUM; i++) {
            messageSendTimes.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            sm.addMember("counter", i);
            sm.addMember("extraData",randomStrings[i]);
            try {
                sendEndpoint->sendMessage(sm);
                auto msg = recvEndpoint->receiveMessage();
                if (!msg->getBoolean("success")) {
                    std::cout << i << " " << msg->stringify() << std::endl;
                    messageReplyTimes.push_back(std::chrono::duration<int64_t, std::nano>::zero());
                }else{
                    messageReplyTimes.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
                }
            } catch (ValidationError &e) {
                std::cerr << sm.stringify() << std::endl;
                std::cout << e.what() << std::endl;
                break;
            } catch (std::logic_error &e) {
                break;
            }
        }

        std::ofstream results;
        results.open("broker_publisher_results.txt", std::fstream::out | std::fstream::app);
        for(auto i : messageSendTimes){
            results << i.count() << ",";
        }
        results << std::endl;
        for(auto i : messageReplyTimes){
            results << i.count() << ",";
        }
        results << std::endl;
        results.close();
        std::cout << "Successfully sent " << i << " messages" << std::endl;
    }

}