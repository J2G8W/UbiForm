#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"

#define USE_RDH false


struct ReceiverData{
    Component & component;
    std::vector<std::chrono::duration<int64_t, std::nano>>& timings;
    bool end = false;
    ReceiverData(Component& c, std::vector<std::chrono::duration<int64_t, std::nano>>& t) : component(c), timings(t) {}
};
void receiverCreationCallback(Endpoint* e, void* data){
    auto userData = static_cast<ReceiverData *>(data);
    // Initial time
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());

    auto message = userData->component.castToPair(e)->receiveMessage();
    // Time to receive the message
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
    userData->end = true;
}

struct SenderData{
    Component & component;
    std::vector<std::chrono::duration<int64_t, std::nano>>& timings;
    SenderData(Component& c, std::vector<std::chrono::duration<int64_t, std::nano>>& t) : component(c), timings(t) {}
};
void senderCreationCallback(Endpoint* e, void* data){
    auto userData = static_cast<SenderData *>(data);
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());

    SocketMessage sm;
    sm.addMember("stringProperty", "Hello");
    sm.addMember("numberProperty", 42);
    std::unique_ptr<SocketMessage> subObject = std::make_unique<SocketMessage>();
    subObject->addMember("subString","world!");
    sm.addMoveObject("objectProperty", std::move(subObject));
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());

    userData->component.castToPair(e)->sendMessage(sm);
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
}


int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0) {
            Component receiver;
            FILE *pFile = fopen("EvaluationFiles/EvaluationManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            receiver.specifyManifest(pFile);
            fclose(pFile);

            receiver.startBackgroundListen();

            std::vector<std::chrono::duration<int64_t, std::nano>> timings;
            auto* data = new ReceiverData(receiver, timings);
            receiver.registerStartupFunction("pairEvaluation", receiverCreationCallback,data);

            while (!data->end) {
                nng_msleep(100);
            }

            std::ofstream results;
            results.open("section_speeds_receiver_results.txt",std::fstream::out | std::fstream::app);
            for(auto x : timings){
                std::cout << x.count() << ",";
            }
            results << std::endl;
            results.close();
        } else if (strcmp(argv[1], SENDER) == 0 && argc >=3) {
            std::vector<std::chrono::duration<int64_t, std::nano>> timings;
            timings.reserve(5);

            // Initial time
            timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            Component sender;
            FILE *pFile = fopen("EvaluationFiles/EvaluationManifest1.json", "r");
            if (pFile == nullptr) throw std::logic_error("Could not find JsonFiles/PairManifest1.json");
            sender.specifyManifest(pFile);
            fclose(pFile);
            auto* data = new SenderData(sender, timings);
            sender.registerStartupFunction("pairEvaluation",senderCreationCallback, data);

            // Time after starting component
            timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            sender.startBackgroundListen();
            sender.getBackgroundRequester().localListenThenRequestRemoteDial(
                    argv[2], "pairEvaluation","pairEvaluation");

            nng_msleep(1000);

            std::ofstream results;
            results.open("section_speeds_sender_results.txt",std::fstream::out | std::fstream::app);
            for(auto x : timings){
                std::cout << x.count() << ",";
            }
            results << std::endl;
            results.close();

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
    }
}