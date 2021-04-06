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
    bool complete = false;
    std::ostream* outputStream;

};

struct startupData{
    Component* component;
    int current = 0;
    timingData* ts;
};

void endOfReceiveStream(PairEndpoint* pe, void* userData){
    auto* t = static_cast<timingData*>(userData);
    t->endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    t->fileSize = t->outputStream->tellp();
    t->complete = true;
    delete t->outputStream;
}

void onEndpointCreation(Endpoint* e, void* userData){
    auto* data = static_cast<startupData*>(userData);
    auto pair = data->component->castToPair(e);
    timingData* td = data->ts + (data->current++);
    td->startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto rcv = pair->receiveStream(*(td->outputStream), endOfReceiveStream, td);
}



int main(int argc, char **argv) {
    if (argc >= 3) {
        Component receiver;
        std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
        std::shared_ptr<EndpointSchema> receiveEndpoint = std::make_shared<EndpointSchema>();
        receiver.getComponentManifest().addEndpoint(ConnectionParadigm::Pair,"receiver",receiveEndpoint,empty);

        int i = std::stoi(argv[2]);
        startupData sd;
        sd.component = &receiver;
        timingData ts[i];
        for(auto &t:ts){
            t.outputStream = new std::ostringstream;
        }
        sd.ts = ts;
        receiver.registerStartupFunction("receiver",onEndpointCreation,&sd);

        for(auto & t : ts) {
            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    argv[1], 8000, "receiver",
                    "sender");
        }

        bool done = false;
        while(!done){
            nng_msleep(500);
            done = true;
            for(auto &t:ts){
                done &= t.complete;
            }
        }
        std::ofstream results;
        results.open("scalability_streaming_results.csv",std::fstream::out | std::fstream::app);
        for(auto &t : ts){
            t.duration = t.endTime - t.startTime;
            results << t.duration.count() << "," << t.fileSize << "," << i << "\n";
        }
        results.close();
    } else {
        std::cerr << "Error usage is " << argv[0] << "  SENDER_ADDRESS  NUMBER\n";
    }
}