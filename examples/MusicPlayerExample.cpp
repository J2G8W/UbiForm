
#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>
#include <SFML/Audio.hpp>


#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


int main(int argc, char **argv) {
    if (argc >= 3) {
        if (strcmp(argv[1], RECEIVER) == 0 ) {
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
            auto endpoints = receiver.getEndpointsByType("receiver");
            if(endpoints->empty()){
                throw std::logic_error("Oops couldn't create connection");
            }

            std::fstream music;
            music.open("temp.wav",std::fstream::binary | std::fstream::out);

            std::shared_ptr<PairEndpoint> pair = receiver.castToPair(endpoints->at(0));


            auto extraInfo = pair->receiveStream(music);
            std::cout << "Extra info: " << extraInfo->getString("extraInfo") << std::endl;

            while (!pair->getReceiverThreadEnded()) {
                nng_msleep(1000);
            }
            music.close();
            sf::SoundBuffer buffer;
            if(!buffer.loadFromFile("temp.wav")){
                throw std::logic_error("Error with file");
            }
            sf::Sound sound;
            sound.setBuffer(buffer);
            sound.play();


            while(true){
                nng_msleep(1000);
            }

        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender;

            std::shared_ptr<EndpointSchema> send = std::make_shared<EndpointSchema>();
            send->addProperty("extraInfo",ValueType::String);
            send->addRequired("extraInfo");
            std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
            sender.getComponentManifest().addEndpoint(SocketType::Pair,"sender",empty,send);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.startBackgroundListen(8000);
            auto endpointVector = sender.getEndpointsByType("sender");
            while (endpointVector->empty()){
                nng_msleep(100);
            }
            auto pair = sender.castToPair(endpointVector->at(0));
            std::ifstream file;

            SocketMessage initalMsg;
            initalMsg.addMember("extraInfo","Stream incoming");

            file.open(argv[2], std::fstream::binary | std::fstream::in);
            if(!file.good()){
                std::cerr << "Unable to open file " << argv[2] << std::endl;
                exit(1);
            }
            pair->sendStream(file, 10002, false, initalMsg);

            while (!pair->getSenderThreadEnded()) {
                nng_msleep(1000);
            }
            file.close();

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
            std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}