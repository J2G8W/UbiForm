#include <algorithm>
#include "../UbiForm/Component.h"
#include "../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <chrono>
#include <iomanip>

#define HUB "HUB"
#define CONNECTION "CONNECTION"

int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], HUB) == 0){
            Component component;

            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;

            component.startBackgroundListen("tcp://127.0.0.1:8000");
            std::cout << "Component Listening" << std::endl;

            component.startResourceDiscoveryHub("tcp://127.0.0.1:7999");

            std::cout << "Resource discovery started" << std::endl;

            ResourceDiscoveryConnEndpoint* rdc = component.createResourceDiscoveryConnectionEndpoint();
            rdc->registerWithHub("tcp://127.0.0.1:7999");

            std::cout << "Registered successfully" << std::endl;


            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component.getSenderEndpointsByType("v1");
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

        }
        if (strcmp(argv[1], CONNECTION) == 0){
            Component component;
            component.startBackgroundListen("tcp://127.0.0.2:8000");

            FILE* pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            ResourceDiscoveryConnEndpoint* rdc = component.createResourceDiscoveryConnectionEndpoint();

            rdc->addResourceDiscoveryHub("tcp://127.0.0.1:7999");

            rdc->registerWithHub("tcp://127.0.0.1:7999");
            std::vector<std::string> ids = rdc->getComponentIdsFromHub("tcp://127.0.0.1:7999");

            std::cout << "Available ids: ";
            for (const auto& i: ids){ std::cout << i << ' ';}
            std::cout << std::endl;

            std::vector<SocketMessage *> newConnection = rdc->getComponentsBySchema("subscriberExample");
            std::cout << "Connectables: ";
            for (const auto& i: newConnection){
                std::cout << i->stringify() << std::endl;
                component.requestAndCreateConnection("subscriberExample",
                                                     i->getString("url"),
                                                     i->getString("endpointType"));
            }
            std::cout << std::endl;


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

