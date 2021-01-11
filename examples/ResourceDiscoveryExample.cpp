#include <algorithm>
#include "../UbiForm/Component.h"
#include "../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>

// Hub is a Publisher and an RDH
#define HUB "HUB"
// Connection is a subscriber
#define SUBSCRIBER_CONNECTION "CONNECTION"
// Publisher is JUST a publisher (different to Hub)
#define PUBLISHER_CONNECTION "PUBLISHER"


int main(int argc, char ** argv){
    const char * componentAddress;
    Component* component;
    if (argc == 4){
        componentAddress = argv[3];
        component = new Component(componentAddress);
    }else{
        component = new Component;
    }
    if (argc >= 3){
        const char * RDHAddress = argv[2];
        if (strcmp(argv[1], HUB) == 0){
            

            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component->specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;

            // Start on a specific port
            component->startBackgroundListen(8000);
            std::cout << "Component Listening" << std::endl;

            component->startResourceDiscoveryHub(7999);

            std::cout << "Resource discovery started" << std::endl;


            component->getResourceDiscoveryConnectionEndpoint().registerWithHub(RDHAddress);

            std::cout << "Registered successfully" << std::endl;


            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component->getSenderEndpointsByType("publisherExample");
            while(true) {
                if (!publisherEndpoints->empty()) {
                    s.addMember("reverse", valid);
                    valid = !valid;

                    char dateString[30];
                    time_t t = time(nullptr);
                    struct tm *p = localtime(&t);
                    strftime(dateString, 30, "%c", p);
                    s.addMember("date", std::string(dateString));

                    publisherEndpoints->at(0)->sendMessage(s);
                }
                nng_msleep(1000);
            }
        }if (strcmp(argv[1], PUBLISHER_CONNECTION) == 0){
            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component->specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;
            // Find some port which works
            component->startBackgroundListen();
            std::cout << "Component Listening" << std::endl;

            component->getResourceDiscoveryConnectionEndpoint().registerWithHub(RDHAddress);

            std::cout << "Registered successfully" << std::endl;


            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component->getSenderEndpointsByType("publisherExample");
            while(true) {
                if (!publisherEndpoints->empty()) {
                    s.addMember("reverse", valid);
                    valid = !valid;

                    char dateString[30];
                    time_t t = time(nullptr);
                    struct tm *p = localtime(&t);
                    strftime(dateString, 30, "%A %B", p);
                    s.addMember("date", std::string(dateString));

                    publisherEndpoints->at(0)->sendMessage(s);
                }
                nng_msleep(1000);
            }
        }
        if (strcmp(argv[1], SUBSCRIBER_CONNECTION) == 0){
            component->startBackgroundListen();

            FILE* pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component->specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";


            const char * locationOfRDH = RDHAddress;



            component->getResourceDiscoveryConnectionEndpoint().registerWithHub(locationOfRDH);
            std::cout << "REGISTERED SUCCESSFULLY" << std::endl;
            std::vector<std::string> ids = component->getResourceDiscoveryConnectionEndpoint().getComponentIdsFromHub(locationOfRDH);

            std::cout << "Available ids: ";
            for (const auto& i: ids){ std::cout << i << ' ';}
            std::cout << std::endl;

            component->getResourceDiscoveryConnectionEndpoint().createEndpointBySchema("subscriberExample");

            std::unique_ptr<SocketMessage> s;
            auto subscriberEndpoints = component->getReceiverEndpointsByType("subscriberExample");
            while(true){
                for (const auto& endpoint : *subscriberEndpoints) {
                    s = endpoint->receiveMessage();

                    std::string date = s->getString("date");
                    if (s->getBoolean("reverse")) {
                        std::cout << date << std::endl;
                    } else {
                        std::reverse(date.begin(), date.end());
                        std::cout << date << std::endl;
                    }
                }
            }
        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << HUB <<"|"<< SUBSCRIBER_CONNECTION << "|" << PUBLISHER_CONNECTION;
        std::cerr << " RDHlocation " << "[componentAddress]" << std::endl;
    }
}

