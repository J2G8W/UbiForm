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
    long fileSize;
    std::ostringstream outputStream;
};

void endOfReceiveStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<timingData*>(userData);
    t->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    t->fileSize = t->outputStream.tellp();
}

struct startupData{
    std::ifstream fileStream;
    Component * component;
    std::string fileName;
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

    t->fileStream.open(t->fileName, std::fstream::binary | std::fstream::in);
    if(!t->fileStream.good()) {
        std::cerr << "Unable to open file " << t->fileName << std::endl;
        exit(1);
    }

    pair->sendStream(t->fileStream, 10002, false, sm, endOfSenderStream, t);
}



int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0 && argc>=3) {
            Component receiver;
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",empty,empty);

            timingData ts[5];

            for(auto & t : ts) {
                t.startTime = std::chrono::high_resolution_clock::now().time_since_epoch();

                receiver.getBackgroundRequester().requestRemoteListenThenDial(
                        argv[2], 8000, "receiver",
                        "sender");

                auto endpoints = receiver.getEndpointsByType("receiver");
                if (endpoints->empty()) {
                    throw std::logic_error("Oops couldn't create connection");
                }

                std::shared_ptr<PairEndpoint> pair = receiver.castToPair(endpoints->at(0));


                pair->receiveStream(t.outputStream, endOfReceiveStream, &t);

                while (!pair->getReceiverThreadEnded()) {
                    nng_msleep(100);
                }
                receiver.closeAndInvalidateSocketById(pair->getEndpointId());
            }
            for(auto &t : ts){
                t.duration = t.endTime - t.startTime;
                std::cout << "Time taken: " << t.duration.count() << "ns\tSize: " << t.fileSize << std::endl;
            }
        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            sender.getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,empty);

            auto* s = new startupData;
            s->fileName = std::string(argv[2]);
            s->component = &sender;
            sender.registerStartupFunction("sender",senderConnectStream,s);

            sender.startBackgroundListen(8000);

            for (int i=0; i<5; i++) {
                while (!s->streamDone) {
                    nng_msleep(100);
                }
                s->streamDone = false;
                sender.closeAndInvalidateSocketsOfType("sender");
                s->fileStream.close();
                //std::cout << i << std::endl;
            }
        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << " SENDER_ADDRESS\n";
            std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}