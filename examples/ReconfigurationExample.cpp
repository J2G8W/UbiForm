
#include "../UbiForm/Component.h"
#include "../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
#include <nng/supplemental/util/platform.h>

// Hub is a Publisher and an RDH
#define HUB "HUB"
// Connection is a subscriber
#define SUBSCRIBER_CONNECTION "CONNECTION"

int main(int argc, char **argv){
    const char * componentAddress;
    if (argc == 4){
        componentAddress = argv[3];
    }else{
        componentAddress = "tcp://127.0.0.1";
    }
    if (argc >= 3) {
        const char *RDHAddress = argv[2];
        if (strcmp(argv[1], HUB) == 0) {

            Component component(componentAddress);

            FILE *pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << std::endl;

            // Start on a specific port
            component.startBackgroundListen(8000);
            std::cout << "Component Listening" << std::endl;

            component.startResourceDiscoveryHub(7999);

            std::cout << "Resource discovery started" << std::endl;



            std::cout << "Registered successfully" << std::endl;


            int counter = 0;
            auto publisherEndpoints = component.getSenderEndpointsByType("publisherExample");
            while (true) {
                auto relevantSchema = component.getComponentManifest().getSenderSchema("publisherExample");
                std::cout << relevantSchema->stringify() << std::endl;
                std::vector<std::string> requiredValues = relevantSchema->getRequired();
                if (!publisherEndpoints->empty()) {
                    SocketMessage sm;
                    for (auto& attributeName : requiredValues){
                        switch(relevantSchema->getValueType(attributeName)){
                            case Number:
                                sm.addMember(attributeName, counter);
                                break;
                            case String:
                                sm.addMember(attributeName,"HELLO");
                                break;
                            case Boolean:
                                sm.addMember(attributeName, true);
                                break;
                            default:
                                throw AccessError("Don't know what to do with non-number/string");
                        }
                    }
                    publisherEndpoints->at(0)->sendMessage(sm);
                }
                counter++;
                if (counter == 4){
                    std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
                    component.getComponentManifest().addEndpoint(SocketType::Publisher, "publisherExample", nullptr,
                                                                  es);
                    component.closeSocketsOfType("publisherExample");
                    component.getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();
                    std::cout << "BORING MANIFEST" << std::endl;
                }
                nng_msleep(1000);
            }
        }if (strcmp(argv[1], SUBSCRIBER_CONNECTION) == 0) {
            Component component(componentAddress);
            component.startBackgroundListen();

            FILE *pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            const char *locationOfRDH = RDHAddress;

            component.getResourceDiscoveryConnectionEndpoint().registerWithHub(locationOfRDH);

            std::vector<std::string> ids = component.getResourceDiscoveryConnectionEndpoint().getComponentIdsFromHub(locationOfRDH);

            std::cout << "Available ids: ";
            for (const auto &i: ids) { std::cout << i << ' '; }
            std::cout << std::endl;

            component.getResourceDiscoveryConnectionEndpoint().createEndpointBySchema("subscriberExample");

            std::unique_ptr<SocketMessage> s;
            auto subscriberEndpoints = component.getReceiverEndpointsByType("subscriberExample");
            while (true) {
                for (const auto &endpoint : *subscriberEndpoints) {
                    s = endpoint->receiveMessage();
                    std::cout << s->stringify() << std::endl;

                }
            }
        }
    }else{
        std::cerr << "Error usage is " << argv[0] << " " << HUB <<"|"<< SUBSCRIBER_CONNECTION ;
        std::cerr << " RDHlocation " << "[componentAddress]" << std::endl;
    }
}