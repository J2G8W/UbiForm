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
    sf::FileInputStream stream;
    sf::Music musicObject;
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
        return;
    }
    if(!userData->musicObject.openFromStream(userData->stream)){
        return;
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
                    //psi->musicObject.stop();
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
        recv->addProperty("endpointNum",ValueType::Number);
        std::shared_ptr<EndpointSchema> empty = std::make_shared<EndpointSchema>();
        receiver.getComponentManifest().addEndpoint(SocketType::Pair,"receiver",recv,empty);

        auto* pairStartupInfo = new PairStartupInfo;
        pairStartupInfo->component = &receiver;
        receiver.registerStartupFunction("receiver", onStreamingEndpointCreation, pairStartupInfo);


        std::thread maintainMusic(maintainMusicPlaying, pairStartupInfo);
        std::vector<std::unique_ptr<SocketMessage>> values;
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
                std::cout << "Option choice ";
                int option;
                std::cin >> option;
                if (values.size() < option){
                    std::cerr << "Not a valid option" << std::endl;
                    continue;
                }
                option -= 1;
                bool found = false;
                for(auto & url : values.at(option)->getArray<std::string>("urls")){
                    try {
                        receiver.getBackgroundRequester().requestRemoteListenThenDial(
                                url, values.at(option)->getInteger("port"),
                                "receiver",
                                values.at(option)->getString("endpointType"));
                        found = true;
                        break;
                    } catch (std::logic_error &e) {
                        std::cout << e.what() << std::endl;
                    }
                }
                if(!found){
                    std::cerr << "Error connecting to music" << std::endl;
                }
            } else if (userInput == "list_options"){
                std::map<std::string, std::string> emptyMap;
                values = receiver.getResourceDiscoveryConnectionEndpoint().getComponentsBySchema("receiver", emptyMap);
                int i=1;
                for(auto& val: values){
                    std::cout << "Option " << i << std::endl;
                    for(auto & url : val->getArray<std::string>("urls")){
                        std::cout << "\t" << url << std::endl;
                    }
                    std::cout << "\tPort: " << val->getInteger("port");
                    std::cout << "\tEndpoint Type: " << val->getString("endpointType") <<std::endl;
                }
            } else if (userInput == "stop"){
                pairStartupInfo->musicObject.stop();
                pairStartupInfo->musicPlaying = false;
            } else{
                std::cout << "Not a valid input" << std::endl;
            }
        }
        std::cout << "Exiting" << std::endl;
        maintainMusic.join();
        delete pairStartupInfo;
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}