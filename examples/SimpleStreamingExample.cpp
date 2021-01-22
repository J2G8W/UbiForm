#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


void receiveOnCreateFunction(std::shared_ptr<DataReceiverEndpoint> receive,
                             std::shared_ptr<DataSenderEndpoint> send, void* extraData){
    std::ofstream f;

    auto t1 = std::chrono::high_resolution_clock::now();
    /*
    if(argc == 3) {
        endpoints->at(0)->receiveStream(std::cout);
    }else{
        f.open(argv[3], std::fstream::binary | std::fstream::out);
        endpoints->at(0)->receiveStream(f);
    }
    while (receive) {
        nng_msleep(1000);
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    if (argc > 3) {
        int fileSize = f.tellp();
        int kiloByteSize = fileSize / 1024;

        std::cout << "Streaming of file of size: " << kiloByteSize << "KB took " << duration << " milliseconds"
                  << std::endl;
    }
    f.close();
     */
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0 && argc>=3) {
            Component receiver;
            std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
            receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",es,es);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    argv[2], 8000, "receiver",
                    "sender");
            auto endpoints = receiver.getReceiverEndpointsByType("receiver");
            while (endpoints->empty()){
                nng_msleep(100);
            }

        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

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
                file.open(argv[2], std::fstream::binary | std::fstream::in);
                if(!file.good()){
                    std::cerr << "Unable to open file " << argv[2] << std::endl;
                    exit(1);
                }
                endpointVector->at(0)->sendStream(file, 10002, false);
            }
            while (!endpointVector->at(0)->getSenderThreadEnded()) {
                nng_msleep(1000);
            }
            file.close();

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS [newFileLocation]\n";
            std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS [newFileLocation]\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}