#include "ResourceDiscoveryStore.h"

class SimpleRDS : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleRDS(): resourceDiscoveryStore(){
        if (pFile == NULL){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        exampleManifest = new ComponentManifest(pFile);
    }

    ~SimpleRDS(){
        delete exampleManifest;
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    ResourceDiscoveryStore resourceDiscoveryStore;
    ComponentManifest * exampleManifest;
};


TEST_F(SimpleRDS,AdditionOfComponent){
    SocketMessage * sm = new SocketMessage;
    sm->addMember("request", ADDITION);

    SocketMessage manifest(exampleManifest->stringify().c_str());
    manifest.addMember("url", "tcp://127.0.0.1:8000");
    sm->addMember("manifest",manifest);


    SocketMessage * returnMsg = nullptr;
    ASSERT_NO_THROW( returnMsg = ResourceDiscoveryStore::generateRDResponse(sm, resourceDiscoveryStore));

    // Assert that we have a returned ID
    ASSERT_NO_THROW(returnMsg->getString("id"));

    delete returnMsg;
    delete sm;
}

TEST_F(SimpleRDS,GetManifestById){
    SocketMessage * sm = new SocketMessage;
    sm->addMember("request", ADDITION);

    std::string listenUrl = "tcp://127.0.0.1:8000";

    SocketMessage manifest(exampleManifest->stringify().c_str());
    manifest.addMember("url", listenUrl);
    sm->addMember("manifest",manifest);


    SocketMessage * returnMsg = nullptr;
    returnMsg = ResourceDiscoveryStore::generateRDResponse(sm, resourceDiscoveryStore);

    // Assert that we have a returned ID
    std::string returnID = returnMsg->getString("id");
    delete returnMsg;


    SocketMessage idRequest;
    idRequest.addMember("request",REQUEST_BY_ID);
    idRequest.addMember("id",returnID);

    returnMsg = ResourceDiscoveryStore::generateRDResponse(&idRequest, resourceDiscoveryStore);

    ASSERT_NO_THROW(returnMsg->isNull("component"));
    ASSERT_FALSE(returnMsg->isNull("component"));


    SocketMessage * componentObject = nullptr;
    ASSERT_NO_THROW(componentObject = returnMsg->getObject("component"));

    ComponentRepresentation componentRepresentation(componentObject);

    ASSERT_EQ(componentRepresentation.getUrl(), listenUrl);
    ASSERT_TRUE(returnMsg->isNull("component"));

    delete sm;
    delete returnMsg;
    delete componentObject;
}