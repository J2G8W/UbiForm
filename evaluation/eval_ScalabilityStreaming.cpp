#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>


struct timingData{
    // Stream start time is also when the endpoint has been created successfully
    std::chrono::duration<int64_t, std::nano> streamStartTime;
    std::chrono::duration<int64_t, std::nano> streamEndTime;
    std::chrono::duration<int64_t, std::nano> streamDuration;
    std::chrono::duration<int64_t, std::nano> endpointCreationStartTime;
    std::chrono::duration<int64_t, std::nano> endpointCreationDuration;
    int endpointNumberAtInit;
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
    t->streamEndTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    t->fileSize = t->outputStream->tellp();
    t->complete = true;
    delete t->outputStream;
}

void onEndpointCreation(Endpoint* e, void* userData){
    auto* data = static_cast<startupData*>(userData);
    auto pair = data->component->castToPair(e);
    timingData* td = data->ts + (data->current++);
    td->streamStartTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto rcv = pair->receiveStream(*(td->outputStream), endOfReceiveStream, td);
    td->endpointNumberAtInit = rcv->getInteger("endpointNum");
}



int main(int argc, char **argv) {
    if (argc >= 3) {
        Component receiver;
        std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
        std::shared_ptr<EndpointSchema> receiveSchema = std::make_shared<EndpointSchema>();
        receiveSchema->addProperty("extraInfo",ValueType::String);
        receiveSchema->addRequired("extraInfo");
        receiveSchema->addProperty("endpointNum",ValueType::Number);
        receiver.getComponentManifest().addEndpoint(ConnectionParadigm::Pair, "receiver", receiveSchema, empty);

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
            t.endpointCreationStartTime = std::chrono::high_resolution_clock::now().time_since_epoch();
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

        int maxNumberOfConnections = 0;
        for (auto  &t: ts){
            if(t.endpointNumberAtInit >maxNumberOfConnections){
                maxNumberOfConnections = t.endpointNumberAtInit;
            }
        }


        std::ofstream streamingResults;
        streamingResults.open("scalability_streaming_results.csv", std::fstream::out | std::fstream::app);
        std::ofstream reconfigResults;
        reconfigResults.open("scalability_reconfiguration_results.csv", std::fstream::out | std::fstream::app);
        for(auto &t : ts){
            t.streamDuration = t.streamEndTime - t.streamStartTime;
            streamingResults << t.streamDuration.count() << "," << t.fileSize << "," << maxNumberOfConnections << "\n";
            t.endpointCreationDuration = t.streamStartTime - t.endpointCreationStartTime;
            reconfigResults << t.endpointCreationDuration.count() << "," << t.endpointNumberAtInit << "\n";
        }
        streamingResults.close();
        reconfigResults.close();
    } else {
        std::cerr << "Error usage is " << argv[0] << "  SENDER_ADDRESS  NUMBER\n";
    }
}