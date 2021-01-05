#include "../UbiForm/Component.h"
#include "../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

// Hub is a Publisher and an RDH
#define HUB "HUB"
// Connection is a subscriber
#define CONNECTION "CONNECTION"

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

            ResourceDiscoveryConnEndpoint *rdc = component.createResourceDiscoveryConnectionEndpoint();
            rdc->registerWithHub(RDHAddress);

            std::cout << "Registered successfully" << std::endl;


            int counter = 0;
            auto publisherEndpoints = component.getSenderEndpointsByType("publisherExample");
            while (true) {
                auto relevantSchema = component.getComponentManifest()->getSenderSchema("publisherExample");
                std::vector<std::string> requiredValues = relevantSchema->getRequired();
                if (!publisherEndpoints->empty()) {
                    SocketMessage sm;
                    for (auto& attributeName : requiredValues){
                        switch(relevantSchema->getValueType(attributeName)){
                            case Number:
                                sm.addMember(attributeName, counter++);
                                break;
                            case String:
                                sm.addMember(attributeName,"HELLO");
                                break;
                            default:
                                throw AccessError("Don't know what to do with non-number/string");
                        }
                    }
                    publisherEndpoints->at(0)->sendMessage(sm);
                }
                if (counter % 2 == 0){
                    std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
                    component.getComponentManifest()->addSchema(SocketType::Publisher, "publisherExample", nullptr, es);
                    rdc->updateManifestWithHubs();
                    std::cout << "BORING MANIFEST" << std::endl;
                }
                sleep(1);
            }
        }
    }else{
        std::cerr << "Error usage is " << argv[0] << " " << HUB <<"|"<< CONNECTION ;
        std::cerr << " RDHlocation " << "[componentAddress]" << std::endl;
    }
}