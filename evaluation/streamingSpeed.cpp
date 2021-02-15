#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"

struct timingData{
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
};

void endOfReceiveStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<timingData*>(userData);
    t->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
}

struct startupData{
    std::ifstream file;
    Component * component;
    bool streamDone = false;
};
void endOfSenderStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<startupData*>(userData);
    t->streamDone = true;
}
void senderConnectStream(Endpoint* e, void* userData){
    auto* t = static_cast<startupData*>(userData);
    SocketMessage sm;
    auto pair = t->component->castToPair(e);
    pair->sendStream(t->file, 10002, false, sm, endOfSenderStream, t);
}



int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0 && argc>=3) {
            Component receiver;
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",empty,empty);

            auto* t = new timingData;
            t->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();


            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    argv[2], 8000, "receiver",
                    "sender");

            auto endpoints = receiver.getEndpointsByType("receiver");
            if(endpoints->empty()){
                throw std::logic_error("Oops couldn't create connection");
            }

            std::shared_ptr<PairEndpoint> pair = receiver.castToPair(endpoints->at(0));

            std::ostringstream outputStream;

            pair->receiveStream(outputStream, endOfReceiveStream, t);

            while (!pair->getReceiverThreadEnded()) {
                nng_msleep(100);
            }

            t->duration = t->endTime - t->startTime;
            std::cout << "Time taken: " << t->duration.count() << "ns" << std::endl;
            delete t;
        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            sender.getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,empty);

            auto* s = new startupData;
            s->component = &sender;
            s->file.open(argv[2], std::fstream::binary | std::fstream::in);
            if(!s->file.good()) {
                std::cerr << "Unable to open file " << argv[2] << std::endl;
                exit(1);
            }
            sender.registerStartupFunction("sender",senderConnectStream,s);

            sender.startBackgroundListen(8000);

            while (!s->streamDone) {
                nng_msleep(1000);
            }
            s->file.close();
        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << " SENDER_ADDRESS\n";
            std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}