#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>

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
            endpoints->at(0)->receiveStream(std::cout);
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
            endpointVector->at(0)->sendStream(std::cin, 3, true);
            while (true) {
                nng_msleep(1000);
            }

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
    }
}