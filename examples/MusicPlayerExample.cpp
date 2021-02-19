#include <cstring>
#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>
#include <fstream>
#include <SFML/Audio.hpp>
#include <SFML/System/FileInputStream.hpp>


#define RECEIVER "RECEIVER"
#define SENDER "SENDER"



struct PairStartupInfo{
    Component* component;
    sf::Music musicObject;
    sf::FileInputStream stream;
    bool progEnd = false;
    bool musicPlaying = false;
    PairStartupInfo() : musicObject(), stream(){}
};

void onPairStreamEnd(PairEndpoint *pEndpoint, void *pVoid){
    auto* musicFileStream = static_cast<std::fstream*>(pVoid);
    musicFileStream->close();
    delete musicFileStream;
}

void onStreamingEndpointCreation(Endpoint* e, void* u){
    auto* userData = static_cast<PairStartupInfo*>(u);

    if (userData->musicPlaying){
        userData->musicObject.stop();
        userData->musicPlaying = false;
    }

    std::string tempFileName;
    tempFileName = "temp" + std::to_string(rand() % 10000) + ".wav";
    auto* musicFileStream = new std::fstream;
    musicFileStream->open(tempFileName, std::fstream::binary | std::fstream::out);
    musicFileStream->flush();

    auto* pair = userData->component->castToPair(e);
    auto extraInfo = pair->receiveStream(*musicFileStream, onPairStreamEnd, musicFileStream);
    std::cout << "Extra info: " << extraInfo->getString("extraInfo") << std::endl;

    nng_msleep(100);
    musicFileStream->flush();
    std::cout << "INITIATING PLAY" << std::endl;


    if (!userData->stream.open(tempFileName)){
        throw std::logic_error("Error with file");
    }
    if(!userData->musicObject.openFromStream(userData->stream)){
        throw std::logic_error("Error with stream");
    }
    userData->musicObject.play();
    userData->musicPlaying = true;
    std::cout << "PLAYING" << std::endl;
}



void maintainMusicPlaying(PairStartupInfo* psi){
    auto recentPos = psi->musicObject.getPlayingOffset();
    while(true){
        if(psi->progEnd){
            break;
        }else if (psi->musicPlaying) {
            if (psi->musicObject.getStatus() == psi->musicObject.Stopped) {
                if (psi->stream.tell() == psi->stream.getSize()) {
                    psi->musicPlaying = false;
                    continue;
                }
                psi->musicObject.play();
                psi->musicObject.setPlayingOffset(recentPos);
                std::cout << "RESET" << std::endl;
            } else {
                recentPos = psi->musicObject.getPlayingOffset();
            }
        }
        nng_msleep(10);
    }
}

int main(int argc, char **argv) {
    if (argc >= 1) {
        Component receiver;
        std::shared_ptr<EndpointSchema> recv = std::make_shared<EndpointSchema>();
        recv->addProperty("extraInfo",ValueType::String);
        recv->addRequired("extraInfo");
        std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
        receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",recv,empty);

        auto* pairStartupInfo = new PairStartupInfo;
        pairStartupInfo->component = &receiver;
        receiver.registerStartupFunction("receiver", onStreamingEndpointCreation, pairStartupInfo);


        std::thread maintainMusic(maintainMusicPlaying, pairStartupInfo);
        while(true) {
            std::string userInput;
            std::cin >> userInput;
            if (userInput == "end") {
                pairStartupInfo->progEnd = true;
                break;
            } else if (userInput.rfind("add_rdh", 0) == 0) {
                std::cout << "Option for Resource Discovery Hub (\"search\" or [uri]): ";
                std::string option;
                std::cin >> option;
                if (option == "search") {
                    std::cout << "Searching for Resource Discovery Hubs" << std::endl;
                    receiver.getResourceDiscoveryConnectionEndpoint().searchForResourceDiscoveryHubs();
                } else {
                    try {
                        receiver.getResourceDiscoveryConnectionEndpoint().registerWithHub(option);
                    } catch (std::logic_error &e) {
                        std::cerr << "Unable to add Resource Discovery Hub at " << option << std::endl;
                    }
                }
            } else if (userInput.rfind("connect", 0) == 0) {
                std::cout << "Dial address: ";
                std::string option;
                std::cin >> option;
                receiver.getBackgroundRequester().requestRemoteListenThenDial(
                        option, 8000, "receiver",
                        "sender");
            }
        }
        std::cout << "End of music reached" << std::endl;
        maintainMusic.join();
        delete pairStartupInfo;
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}