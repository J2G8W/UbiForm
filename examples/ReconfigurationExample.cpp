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
    if (argc >= 2){
        if (strcmp(argv[1], HUB) == 0){
            if (argc == 3){
                componentAddress = argv[2];
                component = new Component(componentAddress);
            }else{
                component = new Component;
            }

            component->startBackgroundListen();

            component->getComponentManifest().setName("RDH");

            component->startResourceDiscoveryHub(7999);


            bool update = false;
            std::string rdhLoc = component->getSelfAddress() + ":" + std::to_string(component->getResourceDiscoveryHubPort());
            std::string idWithRDH = component->getResourceDiscoveryConnectionEndpoint().getId(rdhLoc);
            bool somePublisher = false;
            std::unique_ptr<ComponentRepresentation> publisherRep;
            std::unique_ptr<ComponentRepresentation> subscriberRep;
            int counter = 0;

            while(true){
                if(!update) {
                    for (auto &id : component->getResourceDiscoveryConnectionEndpoint().getComponentIdsFromHub(
                            rdhLoc)) {
                        auto compRep = component->getResourceDiscoveryConnectionEndpoint().getComponentById(rdhLoc, id);
                        if (!somePublisher && compRep != nullptr && compRep->getName() == "Publisher") {
                            somePublisher = true;
                            publisherRep = std::move(compRep);
                            std::cout << "Found Publisher" << std::endl;
                        }else if (compRep != nullptr && somePublisher && compRep->getName() == "Subscriber"){
                            for(const auto& url:compRep->getAllUrls()){
                                std::string dialUrl = url + ":" + std::to_string(compRep->getPort());
                                component->getBackgroundRequester().requestChangeEndpoint(dialUrl,SocketType::Subscriber,
                                                                                          "subscriberExample",
                                                                                          publisherRep->getSenderSchema("publisherExample").get(),nullptr);
                                std::cout << "Subscriber component added endpoint schema" << std::endl;
                                component->getBackgroundRequester().tellToRequestAndCreateConnection(dialUrl,"subscriberExample",
                                                                                                     "publisherExample",publisherRep->getAllUrls().at(0),
                                                                                                     publisherRep->getPort());
                                std::cout << "Subscriber has formed connection to publisher" << std::endl;
                                update = true;
                                break;
                            }
                            subscriberRep = std::move(compRep);
                        }
                    }
                }else{
                    if(counter++ == 3){
                        for(const auto& url:subscriberRep->getAllUrls()){
                            std::string dialUrl = url + ":" + std::to_string(subscriberRep->getPort());
                            component->getBackgroundRequester().requestCloseSocketOfType(dialUrl,"subscriberExample");
                        }
                        std::cout << "Subscriber socket closed" << std::endl;

                        for(const auto& url:publisherRep->getAllUrls()){
                            std::string dialUrl = url + ":" + std::to_string(publisherRep->getPort());
                            component->getBackgroundRequester().requestCloseSocketOfType(dialUrl,"publisherExample");
                        }
                        std::cout << "Publisher socket closed" << std::endl;
                    }
                }
                nng_msleep(1000);
            }


        }else if (argc >=3){
            const char *RDHAddress = argv[2];
            if (argc == 4){
                componentAddress = argv[3];
                component = new Component(componentAddress);
            }else{
                component = new Component;
            }

            if (strcmp(argv[1], PUBLISHER_CONNECTION) == 0) {
                FILE *pFile = fopen("JsonFiles/PublisherManifest2.json", "r");
                if (pFile == nullptr) perror("ERROR");
                component->specifyManifest(pFile);
                fclose(pFile);
                std::cout << "Publisher Manifest Specified" << std::endl;

                component->startBackgroundListen();

                while(true) {
                    try {
                        component->getResourceDiscoveryConnectionEndpoint().registerWithHub(RDHAddress);
                        break;
                    } catch (std::logic_error &e) {
                        nng_msleep(1000);
                    }
                }


                SocketMessage s;
                int counter = 1;
                auto publisherEndpoints = component->getSenderEndpointsByType("publisherExample");
                while (true) {
                    if (!publisherEndpoints->empty()) {
                        char dateString[30];
                        time_t t = time(nullptr);
                        struct tm *p = localtime(&t);
                        strftime(dateString, 30, "%A %B", p);
                        s.addMember("date", std::string(dateString));

                        s.addMember("messagesSent",1);
                        s.addMember("counter",counter++);

                        publisherEndpoints->at(0)->sendMessage(s);
                    }
                    nng_msleep(1000);
                }
            }
            if (strcmp(argv[1], SUBSCRIBER_CONNECTION) == 0) {
                component->startBackgroundListen();
                component->getComponentManifest().setName("Subscriber");

                const char *locationOfRDH = RDHAddress;

                component->getResourceDiscoveryConnectionEndpoint().registerWithHub(locationOfRDH);

                std::unique_ptr<SocketMessage> s;
                auto subscriberEndpoints = component->getReceiverEndpointsByType("subscriberExample");

                std::map<std::string, int> counters;
                while (true) {
                    for (const auto &endpoint : *subscriberEndpoints) {
                        s = endpoint->receiveMessage();
                        std::cout << "Receiving from: " << endpoint->getDialUrl() << std::endl;
                        for(const std::string& name: component->getComponentManifest().getReceiverSchema("subscriberExample")->getRequired()){
                            ValueType type = component->getComponentManifest().getReceiverSchema("subscriberExample")->getValueType(name);
                            switch (type) {
                                case Number: {
                                    auto it = counters.find(name);
                                    if (it != counters.end()) {
                                        it->second += s->getInteger(name);
                                    } else {
                                        counters.insert(std::make_pair(name, s->getInteger(name)));
                                    }
                                    std::cout << "\tTallyed value of " << name << " : " << counters.at(name) << std::endl;
                                    break;
                                }
                                case String:
                                    std::cout << "\tReceived " << name << " : " << s->getString(name) <<std::endl;
                                    break;
                                default:
                                    std::cout << "\tUnsure how to handle " << name << " of type " <<  convertValueType(type) <<std::endl;
                            }
                        }
                    }
                    nng_msleep(1000);
                }
            }
        }
    }

    std::cerr << "Error usage is " << argv[0] << " " << HUB  << "[componentAddress]" << std::endl;
    std::cerr << "or: " << argv[0] << " " << SUBSCRIBER_CONNECTION << "|" << PUBLISHER_CONNECTION;
    std::cerr << " RDHlocation " << "[componentAddress]" << std::endl;

}

