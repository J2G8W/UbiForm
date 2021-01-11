#include <cstring>

#include "../UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"

int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], RECEIVER) == 0){
            Component receiver("tcp://127.0.0.2");
            FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            receiver.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestAndCreateConnection(
                    "tcp://127.0.0.1:8000", -1, "pairExample",
                    "pairExample");
            auto endpoints = receiver.getReceiverEndpointsByType("pairExample");
            while(true){
                for(const auto& e: *endpoints){
                    auto msg = e->receiveMessage();
                    std::cout << msg->getInteger("temp") << std::endl;
                }
            }

        }
        if (strcmp(argv[1], SENDER) == 0){
            Component sender("tcp://127.0.0.2");

            FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            sender.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.startBackgroundListen(8000);
            auto endpointVector = sender.getSenderEndpointsByType("pairExample");
            int i =0;
            while(true){
                nng_msleep(1000);
                try {
                    SocketMessage sm;
                    sm.addMember("temp", i++);
                    sm.addMember("msg", std::string("HELLO WORLD!"));
                    for (const auto& e : *endpointVector){
                        e->asyncSendMessage(sm);
                    }

                }catch (std::out_of_range &e){
                    std::cout << "ERROR" << std::endl;
                    // IGNORED
                }
            }

        }else{
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
        }
    }else{
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
    }
}