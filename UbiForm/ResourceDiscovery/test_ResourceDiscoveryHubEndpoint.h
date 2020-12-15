#include "ResourceDiscoveryHubEndpoint.h"

class SimpleRDHE : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleRDHE(){
        rdhe = new ResourceDiscoveryHubEndpoint("ipc:///tmp/test.ipc");
        if (pFile == NULL){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        exampleManifest = new ComponentManifest(pFile);
    }

    ~SimpleRDHE(){
        //delete rdhe;
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    ResourceDiscoveryHubEndpoint* rdhe;
    ComponentManifest * exampleManifest;
};


TEST_F(SimpleRDHE,AdditionOfComponent){
    SocketMessage * sm = new SocketMessage;
    sm->addMember("request", ADDITION);

}