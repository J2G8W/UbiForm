#include <algorithm>
#include "../UbiForm/Component.h"
#include "../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <chrono>
#include <iomanip>

// Hub is a Publisher and an RDH
#define HUB "HUB"
// Connection is a subscriber
#define CONNECTION "CONNECTION"
// Publisher is JUST a publisher (different to Hub)
#define PUBLISHER "PUBLISHER"

#define RDHlocation "tcp://127.0.0.1:7999"

int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], HUB) == 0){
            Component component("tcp://127.0.0.1");

            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;

            component.startBackgroundListen(8000);
            std::cout << "Component Listening" << std::endl;

            component.startResourceDiscoveryHub(7999);

            std::cout << "Resource discovery started" << std::endl;

            ResourceDiscoveryConnEndpoint* rdc = component.createResourceDiscoveryConnectionEndpoint();
            rdc->registerWithHub(RDHlocation);

            std::cout << "Registered successfully" << std::endl;


            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component.getSenderEndpointsByType("publisherExample");
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
                sleep(1);
            }
        }if (strcmp(argv[1], PUBLISHER) == 0){
            Component component("tcp://127.0.0.3");

            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;

            component.startBackgroundListen();
            std::cout << "Component Listening" << std::endl;

            ResourceDiscoveryConnEndpoint* rdc = component.createResourceDiscoveryConnectionEndpoint();
            rdc->registerWithHub(RDHlocation);

            std::cout << "Registered successfully" << std::endl;


            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component.getSenderEndpointsByType("publisherExample");
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
                sleep(1);
            }
        }
        if (strcmp(argv[1], CONNECTION) == 0){
            Component component("tcp://127.0.0.2");
            component.startBackgroundListen();

            FILE* pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";


            const char * locationOfRDH = RDHlocation;

            ResourceDiscoveryConnEndpoint* rdc = component.createResourceDiscoveryConnectionEndpoint();
            rdc->addResourceDiscoveryHub(locationOfRDH);

            rdc->registerWithHub(locationOfRDH);
            std::vector<std::string> ids = rdc->getComponentIdsFromHub(locationOfRDH);

            std::cout << "Available ids: ";
            for (const auto& i: ids){ std::cout << i << ' ';}
            std::cout << std::endl;

            rdc->createEndpointBySchema("subscriberExample");

            std::unique_ptr<SocketMessage> s;
            auto subscriberEndpoints = component.getReceiverEndpointsByType("subscriberExample");
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
        std::cerr << "Error usage is " << argv[0] << " " << HUB <<"|"<< CONNECTION << "\n";
    }
}

