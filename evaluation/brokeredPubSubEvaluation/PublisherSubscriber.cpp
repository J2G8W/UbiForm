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

int main(int argc, char **argv) {
    if (strcmp(argv[1], SUBSCRIBER_COMPONENT) == 0 && argc >= 3) {
        Component component;

        std::shared_ptr <EndpointSchema> es = std::make_shared<EndpointSchema>();
        es->addProperty("counter", ValueType::Number);
        es->addRequired("counter");
        component.getComponentManifest().addEndpoint(SocketType::Subscriber, "speedSub", es, nullptr);

        auto *userData = new subscriberStartupData;
        userData->component = &component;
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
        results.open("results.txt", std::fstream::out | std::fstream::app);
        results << userData->duration.count() << "," << userData->messagesReceived << "," << userData->messagesLost
                << "\n";
        results.close();
    } else if (strcmp(argv[1], PUBLISHER_COMPONENT) == 0) {
        Component component;

        std::shared_ptr <EndpointSchema> sendSchema = std::make_shared<EndpointSchema>();
        sendSchema->addProperty("counter", ValueType::Number);
        sendSchema->addRequired("counter");

        std::shared_ptr <EndpointSchema> receiveSchema = std::make_shared<EndpointSchema>();
        receiveSchema->addProperty("success", ValueType::Boolean);
        receiveSchema->addRequired("success");
        component.getComponentManifest().addEndpoint(SocketType::Request, "dataPublisher",
                                                     receiveSchema, sendSchema);

        component.startBackgroundListen();


        std::string brokerAddress = argv[2];
        component.getBackgroundRequester().requestChangeEndpoint(brokerAddress+":8000", SocketType::Reply,
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


        SocketMessage sm;
        int i = 0;
        for (; i < MESSAGE_NUM; i++) {
            sm.addMember("counter", i);
            try {
                sendEndpoint->sendMessage(sm);
                auto msg = recvEndpoint->receiveMessage();
                if (!msg->getBoolean("success")) {
                    std::cout << i << " " << msg->stringify() << std::endl;
                }
            } catch (ValidationError &e) {
                std::cerr << sm.stringify() << std::endl;
                std::cout << e.what() << std::endl;
                break;
            } catch (std::logic_error &e) {
                break;
            }
        }

        std::cout << "Successfully sent " << i << " messages" << std::endl;
    }

}