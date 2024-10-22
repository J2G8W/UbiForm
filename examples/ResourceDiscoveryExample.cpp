#include <algorithm>
#include "../include/UbiForm/Component.h"
#include "../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>

// Hub is a Publisher and an RDH
#define HUB "HUB"
// Connection is a subscriber
#define SUBSCRIBER_CONNECTION "CONNECTION"
// Publisher is JUST a publisher (different to Hub)
#define PUBLISHER_CONNECTION "PUBLISHER"



void subscriberStartupFunction(Endpoint* endpoint, void *extraInfo) {
    auto *component = static_cast<Component*>(extraInfo);
    auto * subscriber = component->castToDataReceiverEndpoint(endpoint);
    while (true){
        try{
            auto sm = subscriber->receiveMessage();
            std::string date = sm->getString("date");
            if (sm->getBoolean("reverse")) {
                std::cout << date << std::endl;
            } else {
                std::reverse(date.begin(), date.end());
                std::cout << date << std::endl;
            }
        }catch (std::logic_error& e){
            break;
        }
    }
}

int main(int argc, char **argv) {
    const char *componentAddress;
    Component *component;
    if (argc >= 2) {
        if (strcmp(argv[1], HUB) == 0) {
            if (argc == 3) {
                componentAddress = argv[2];
                component = new Component(componentAddress);
            } else {
                component = new Component;
            }

            FILE *pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component->specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << std::endl;

            // Start on a specific port
            component->startBackgroundListen(8000);
            if(VIEW_STD_OUTPUT) std::cout << "Component Listening" << std::endl;

            component->startResourceDiscoveryHub(7999);

            if(VIEW_STD_OUTPUT) std::cout << "Resource discovery started" << std::endl;


            EndpointMessage s;
            bool valid = true;
            auto publisherEndpoints = component->getEndpointsByType("publisherExample");
            while (true) {
                if (!publisherEndpoints->empty()) {
                    s.addMember("reverse", valid);
                    valid = !valid;

                    char dateString[30];
                    time_t t = time(nullptr);
                    struct tm *p = localtime(&t);
                    strftime(dateString, 30, "%c", p);
                    s.addMember("date", std::string(dateString));

                    component->castToDataSenderEndpoint(publisherEndpoints->at(0))->sendMessage(s);
                }
                nng_msleep(1000);
            }
        } else {

            if (argc >= 3) {
                componentAddress = argv[2];
                component = new Component(componentAddress);
            } else {
                component = new Component;
            }

            if (strcmp(argv[1], PUBLISHER_CONNECTION) == 0) {
                FILE *pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
                if (pFile == nullptr) perror("ERROR");
                component->specifyManifest(pFile);
                fclose(pFile);

                if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << std::endl;
                // Find some port which works
                component->startBackgroundListen();
                if(VIEW_STD_OUTPUT) std::cout << "Component Listening" << std::endl;

                if (argc < 4) {
                    if(VIEW_STD_OUTPUT) std::cout << "Search for Resource Discovery Hubs" << std::endl;
                    component->getResourceDiscoveryConnectionEndpoint().searchForResourceDiscoveryHubs();
                } else {
                    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(argv[3]);
                }


                EndpointMessage s;
                bool valid = true;
                auto publisherEndpoints = component->getEndpointsByType("publisherExample");
                while (true) {
                    if (!publisherEndpoints->empty()) {
                        s.addMember("reverse", valid);
                        valid = !valid;

                        char dateString[30];
                        time_t t = time(nullptr);
                        struct tm *p = localtime(&t);
                        strftime(dateString, 30, "%A %B", p);
                        s.addMember("date", std::string(dateString));

                        component->castToDataSenderEndpoint(publisherEndpoints->at(0))->sendMessage(s);
                    }
                    nng_msleep(1000);
                }
            }
            if (strcmp(argv[1], SUBSCRIBER_CONNECTION) == 0) {
                component->startBackgroundListen();

                FILE *pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
                if (pFile == nullptr) perror("ERROR");
                component->specifyManifest(pFile);
                fclose(pFile);

                if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";


                if (argc < 4) {
                    if(VIEW_STD_OUTPUT) std::cout << "Search for Resource Discovery Hubs" << std::endl;
                    component->getResourceDiscoveryConnectionEndpoint().searchForResourceDiscoveryHubs();
                } else {
                    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(argv[3]);
                }
                auto RDHurls = component->getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs();
                if (RDHurls.empty()) {
                    std::cerr << "No Resource Discovery Hubs found" << std::endl;
                    return -1;
                }
                std::string locationOfRDH = RDHurls.at(0);
                std::vector<std::string> ids = component->getResourceDiscoveryConnectionEndpoint().getComponentIdsFromHub(
                        locationOfRDH);

                std::cout << "Available ids: ";
                for (const auto &i: ids) { std::cout << i << ' '; }
                std::cout << std::endl;

                component->registerStartupFunction("subscriberExample",subscriberStartupFunction,component);

                component->getResourceDiscoveryConnectionEndpoint().createEndpointBySchema("subscriberExample");

                std::unique_ptr<EndpointMessage> s;
                auto subscriberEndpoints = component->getEndpointsByType("subscriberExample");

                int counter = 1;

                while (true) {
                    if(counter % 4 == 0){
                        std::cout << "Refreshing endpoints" << std::endl;
                        component->closeAndInvalidateEndpointsOfType("subscriberExample");
                        component->getResourceDiscoveryConnectionEndpoint().createEndpointBySchema("subscriberExample");
                    }
                    nng_msleep(3000);
                    counter++;
                }
            }
        }
    }

    std::cerr << "Error usage is " << argv[0] << " " << HUB << "[componentAddress]" << std::endl;
    std::cerr << "or: " << argv[0] << " " << SUBSCRIBER_CONNECTION << "|" << PUBLISHER_CONNECTION;
    std::cerr << "[componentAddress]" << " [RDHlocation] " << std::endl;

}

