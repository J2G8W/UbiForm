#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0 && argc>=3) {
            Component receiver;
            std::shared_ptr<EndpointSchema> recv = std::make_shared<EndpointSchema>();
            recv->addProperty("extraInfo",ValueType::String);
            recv->addRequired("extraInfo");
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",recv,empty);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    argv[2], 8000, "receiver",
                    "sender");
            auto endpoints = receiver.getReceiverEndpointsByType("receiver");
            if(endpoints->empty()){
                throw std::logic_error("Oops couldn't create connection");
            }

            std::ofstream f;

            auto t1 = std::chrono::high_resolution_clock::now();
            std::shared_ptr<PairEndpoint> pair = receiver.castToPair(endpoints->at(0));
            if(argc == 3) {
                auto extraInfo = pair->receiveStream(std::cout);
                std::cout << "Extra info: " << extraInfo->getString("extraInfo") << std::endl;
            }else{
                f.open(argv[3], std::fstream::binary | std::fstream::out);
                auto extraInfo = pair->receiveStream(f);
                std::cout << "Extra info: " << extraInfo->getString("extraInfo") << std::endl;
            }
            while (!pair->getReceiverThreadEnded()) {
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

        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

            std::shared_ptr<EndpointSchema> send = std::make_shared<EndpointSchema>();
            send->addProperty("extraInfo",ValueType::String);
            send->addRequired("extraInfo");
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            sender.getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,send);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.startBackgroundListen(8000);
            auto endpointVector = sender.getSenderEndpointsByType("sender");
            while (endpointVector->empty()){
                nng_msleep(100);
            }
            auto pair = sender.castToPair(endpointVector->at(0));
            std::ifstream file;

            SocketMessage initalMsg;
            initalMsg.addMember("extraInfo","Stream incoming");
            if(argc == 2) {
                pair->sendStream(std::cin, 3, true, initalMsg);
            }else{
                file.open(argv[2], std::fstream::binary | std::fstream::in);
                if(!file.good()){
                    std::cerr << "Unable to open file " << argv[2] << std::endl;
                    exit(1);
                }
                pair->sendStream(file, 10002, false, initalMsg);
            }
            while (!pair->getSenderThreadEnded()) {
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