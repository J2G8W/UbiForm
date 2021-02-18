#include <algorithm>
#include "../include/UbiForm/Component.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>

#define SUBSCRIBER_COMPONENT "SUBSCRIBER"
#define PUBLISHER_COMPONENT "PUBLISHER"
#define MESSAGE_NUM 10000

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
    userData->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto msg = subscriber->receiveMessage();
    int localCounter = msg->getInteger("counter") + 1;
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
    userData->messagesReceived = localCounter - userData->messagesLost;
    userData->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    userData->done = true;
}

struct publisherStartupData{
    Component* component;
    bool done = false;
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
    int messagesSent;
};
void publisherStartup(Endpoint * e, void * d){
    auto* userData = static_cast<publisherStartupData*>(d);
    DataSenderEndpoint* publisher = userData->component->castToDataSenderEndpoint(e);
    SocketMessage sm;
    userData->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    int i = 0;
    for ( ; i < MESSAGE_NUM ; i++){
        sm.addMember("counter",i);
        try{
            publisher->sendMessage(sm);
        }catch (ValidationError &e){
            std::cerr << sm.stringify() << std::endl;
        }catch (std::logic_error &e) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    userData->messagesSent = i;
    userData->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    userData->done = true;
}


int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], SUBSCRIBER_COMPONENT) == 0 && argc>=3) {
            Component component;

            std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
            es->addProperty("counter", ValueType::Number);
            es->addRequired("counter");
            component.getComponentManifest().addEndpoint(SocketType::Subscriber,"speedSub",es, nullptr);

            auto* userData = new subscriberStartupData;
            userData->component = &component;
            component.registerStartupFunction("speedSub", subscriberStartup, userData);

            component.getBackgroundRequester().requestRemoteListenThenDial(
                    argv[2], 8000, "speedSub",
                    "speedPub");

            while(!userData->done){
                nng_msleep(500);
            }
            userData->duration = userData->endTime - userData->startTime;
            std::cout << userData->duration.count() << "," << userData->messagesReceived << "," << userData->messagesLost<< "\n";
        }
        else if (strcmp(argv[1], PUBLISHER_COMPONENT) == 0) {
            Component component;

            std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
            es->addProperty("counter", ValueType::Number);
            es->addRequired("counter");
            component.getComponentManifest().addEndpoint(SocketType::Publisher,"speedPub",nullptr, es);


            component.startBackgroundListen(8000);
            auto* userData = new publisherStartupData;
            userData->component = &component;
            component.registerStartupFunction("speedPub",publisherStartup, userData);

            while (!userData->done){
                nng_msleep(500);
            }

            userData->duration = userData->endTime - userData->startTime;
            std::cout << userData->duration.count() << "," << userData->messagesSent << "\n";
        }else {
            std::cerr << "Error usage is " << argv[0] << " " << PUBLISHER_COMPONENT << "\n"<< argv[0] << SUBSCRIBER_COMPONENT << "dialAddress" <<  std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << PUBLISHER_COMPONENT << "\n"<< argv[0] << SUBSCRIBER_COMPONENT << "dialAddress" <<  std::endl;
    }
}

