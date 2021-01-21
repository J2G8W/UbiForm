#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0) {
            Component receiver("tcp://127.0.0.2");
            std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
            receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",es,es);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    "tcp://127.0.0.1", 8000, "receiver",
                    "sender");
            auto endpoints = receiver.getReceiverEndpointsByType("receiver");
            while (endpoints->empty()){
                nng_msleep(100);
            }
            std::ofstream f;
            if(argc == 2) {
                endpoints->at(0)->receiveStream(std::cout);
            }else{
                f.open(argv[3]);
                endpoints->at(0)->receiveStream(f);
            }
            while (true) {
                nng_msleep(1000);
            }

        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender("tcp://127.0.0.1");

            std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();;
            sender.getComponentManifest().addEndpoint(SocketType::Pair,"sender",es,es);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.startBackgroundListen(8000);
            auto endpointVector = sender.getSenderEndpointsByType("sender");
            while (endpointVector->empty()){
                nng_msleep(100);
            }
            std::ifstream file;
            if(argc == 2) {
                endpointVector->at(0)->sendStream(std::cin, 3, true);
            }else{
                file.open(argv[2]);
                endpointVector->at(0)->sendStream(file, 10002, false);
            }
            while (!endpointVector->at(0)->getSenderThreadEnded()) {
                nng_msleep(1000);
            }

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
    }
}