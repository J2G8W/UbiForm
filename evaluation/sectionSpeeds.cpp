#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"

#define USE_RDH false
#define USE_ASYNC_SEND false


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

    EndpointMessage sm;
    sm.addMember("stringProperty", "Hello");
    sm.addMember("numberProperty", 42);
    std::unique_ptr<EndpointMessage> subObject = std::make_unique<EndpointMessage>();
    subObject->addMember("subString","world!");
    sm.addMoveObject("objectProperty", std::move(subObject));
    userData->timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());

    if (USE_ASYNC_SEND){
        userData->component.castToPair(e)->asyncSendMessage(sm);
    } else{
        userData->component.castToPair(e)->sendMessage(sm);
    }

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

            if (argc >= 3){
                receiver.getResourceDiscoveryConnectionEndpoint().registerWithHub(argv[2]);
            }

            std::vector<std::chrono::duration<int64_t, std::nano>> timings;
            auto* data = new ReceiverData(receiver, timings);
            receiver.registerStartupFunction("pairEvaluation", receiverCreationCallback,data);

            while (!data->end) {
                nng_msleep(100);
            }

            std::ofstream results;
            results.open("section_speeds_receiver_results.csv",std::fstream::out | std::fstream::app);
            // Inital time (when receiver made), Time after receiving message
            for(auto x : timings){
                results << x.count() << ",";
            }
            results << std::endl;
            results.close();

            if (argc >= 3){
                receiver.getResourceDiscoveryConnectionEndpoint().deRegisterFromAllHubs();
            }
        } else if (strcmp(argv[1], SENDER) == 0 && argc >=3) {
            std::vector<std::chrono::duration<int64_t, std::nano>> timings;
            timings.reserve(7);

            // Initial time
            timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            Component sender;
            FILE *pFile = fopen("EvaluationFiles/EvaluationManifest1.json", "r");
            if (pFile == nullptr) throw std::logic_error("Could not find JsonFiles/PairManifest1.json");
            sender.specifyManifest(pFile);
            fclose(pFile);
            auto* data = new SenderData(sender, timings);
            sender.registerStartupFunction("pairEvaluation",senderCreationCallback, data);

            sender.startBackgroundListen();

            // Time after starting component
            timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            std::string dialAddress = argv[2];
            if (USE_RDH){
                sender.getResourceDiscoveryConnectionEndpoint().registerWithHub(argv[2]);
                timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
                std::map<std::string, std::string> empty;
                auto receivers = sender.getResourceDiscoveryConnectionEndpoint().getComponentsBySchema("pairEvaluation", empty);
                timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
                if(receivers.empty()){throw std::logic_error("No receivers returned");}
                dialAddress = receivers.at(0)->getArray<std::string>("urls").at(0);
                dialAddress += ":" + std::to_string(receivers.at(0)->getInteger("port"));

            }else{
                timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
                timings.push_back(std::chrono::high_resolution_clock::now().time_since_epoch());
            }

            sender.getBackgroundRequester().localListenThenRequestRemoteDial(
                   dialAddress , "pairEvaluation","pairEvaluation");



            nng_msleep(1000);

            std::ofstream results;
            results.open("section_speeds_sender_results.csv",std::fstream::out | std::fstream::app);
            // Initial Time, Time after component start, Time after register with hub, Time after request for components,
            //      Time at start of endpoint, Time after making message, Time after sending message
            for(auto x : timings){
                results << x.count() << ",";
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