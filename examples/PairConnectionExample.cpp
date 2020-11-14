#include <cstring>
#include <unistd.h>

#include "nng/nng.h"
#include "nng/protocol/pair1/pair.h"

#include "../UbiForm/Component.h"
#define RECEIVER "RECEIVER"
#define SENDER "SENDER"



int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], RECEIVER) == 0){
            Component receiver;

            receiver.specifyManifest(R"({"name":"RECEIVER"})");
            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.createPairConnectionIncoming("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            receiver.receiveManifestOnSocket();
            std::cout << "MANIFEST RECEIVED" << "\n";
        }
        if (strcmp(argv[1], SENDER) == 0){
            Component sender;

            sender.specifyManifest(R"({"name":"SENDER"})");
            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.createPairConnectionOutgoing("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            sender.sendManifestOnSocket();
            std::cout << "MANIFEST SENT" << "\n";

        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
    }
}

