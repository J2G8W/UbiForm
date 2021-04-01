#include <cstring>

#include "../../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


struct timingData{
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
    long fileSize;
    int blockSize;
    std::ostream* outputStream;
};

void endOfReceiveStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<timingData*>(userData);
    t->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    t->fileSize = t->outputStream->tellp();
    delete t->outputStream;
}

struct startupData{
    std::ifstream fileStream;
    Component * component;
    std::string fileName;
    bool streamDone = false;
    int blockSize;
};
void endOfSenderStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<startupData*>(userData);
    t->fileStream.close();
    t->streamDone = true;
}
void senderConnectStream(Endpoint* e, void* userData){
    auto* t = static_cast<startupData*>(userData);
    EndpointMessage sm;
    sm.addMember("blockSize", t->blockSize);
    auto pair = t->component->castToPair(e);

    t->fileStream.open(t->fileName, std::fstream::binary | std::fstream::in);
    if(!t->fileStream.good()) {
        std::cerr << "Unable to open file " << t->fileName << std::endl;
    }

    pair->sendStream(t->fileStream, t->blockSize, false, sm, endOfSenderStream, t);
}



int main(int argc, char **argv) {
    if (argc >= 3) {
        if (strcmp(argv[1], RECEIVER) == 0) {
            Component receiver;
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            std::shared_ptr<EndpointSchema> receiveEndpoint = std::make_shared<EndpointSchema>();
            receiveEndpoint->addProperty("blockSize",ValueType::Number);
            receiveEndpoint->addRequired("blockSize");
            receiver.getComponentManifest().addEndpoint(ConnectionParadigm::Pair,"receiver",receiveEndpoint,empty);

            timingData ts[5];
            for(auto &t:ts){
                t.outputStream = new std::ostringstream;
            }

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


                auto rcv = pair->receiveStream(*(t.outputStream), endOfReceiveStream, &t);
                t.blockSize = rcv->getInteger("blockSize");

                while (!pair->getReceiverThreadEnded()) {
                    nng_msleep(100);
                }
                receiver.closeAndInvalidateEndpointsById(pair->getEndpointId());
                nng_msleep(500);
            }
            std::ofstream results;
            results.open("streaming_UbiForm_results.csv",std::fstream::out | std::fstream::app);
            for(auto &t : ts){
                t.duration = t.endTime - t.startTime;
                results << t.duration.count() << "," << t.fileSize << "," << t.blockSize << "\n";
            }
            results.close();
        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            std::shared_ptr<EndpointSchema> senderSchema = std::make_shared<EndpointSchema>();
            senderSchema->addProperty("blockSize",ValueType::Number);
            senderSchema->addRequired("blockSize");
            sender.getComponentManifest().addEndpoint(ConnectionParadigm::Pair,"sender",empty,senderSchema);

            auto* s = new startupData;
            s->fileName = std::string(argv[2]);
            s->component = &sender;
            if(argc >=4 ){
                try {
                    s->blockSize = std::stoi(argv[3]);
                }catch (std::logic_error &e){
                    std::cerr << e.what() << std::endl;
                    exit(-1);
                }
            } else{
                s->blockSize = 50001;
            }
            sender.registerStartupFunction("sender",senderConnectStream,s);

            sender.startBackgroundListen(8000);

            for (int i=0; i<5; i++) {
                while (!s->streamDone) {
                    nng_msleep(100);
                }
                s->streamDone = false;
                sender.closeAndInvalidateEndpointsOfType("sender");
            }

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << " SENDER_ADDRESS\n";
            std::cerr << argv[0] << " " << SENDER << " fileLocation " <<  " [blockSize]" << std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << " SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " fileLocation " <<  " [blockSize]" << std::endl;
    }
}