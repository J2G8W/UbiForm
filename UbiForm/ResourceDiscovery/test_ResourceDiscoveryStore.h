#include "ResourceDiscoveryStore.h"

#include <algorithm>
#include <fstream>

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

    SocketMessage* addDummyComponent(std::string listenUrl){
        SocketMessage sm;
        sm.addMember("request", ADDITION);

        SocketMessage manifest(exampleManifest->stringify().c_str());
        manifest.addMember("url", listenUrl);
        sm.addMember("manifest",manifest);


        SocketMessage * returnMsg = nullptr;
        returnMsg = ResourceDiscoveryStore::generateRDResponse(&sm, resourceDiscoveryStore);

        return returnMsg;
    }

    SocketMessage * loadSocketMessage(std::string location){
        try {
            std::ifstream in(location);
            std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            SocketMessage *socketMessage = new SocketMessage(contents.c_str());
            return socketMessage;
        }catch(const std::ifstream::failure &e){
            std::cerr << "ERROR OPENING FILE: " << location <<std::endl;
            throw;
        }
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
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
    std::string listenUrl = "tcp://127.0.0.1:8000";
    SocketMessage * returnMsg = addDummyComponent(listenUrl);

    // Assert that we have a returned ID
    std::string returnID = returnMsg->getString("id");

    // We delete the returned message because it is safe to do so
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

    delete returnMsg;
    delete componentObject;
}

TEST_F(SimpleRDS, GetComponentIds){
    std::string url1 = "tcp://127.0.0.1:8000";
    std::string url2 = "tcp://127.0.0.2:8001";

    SocketMessage * returnMsg = addDummyComponent(url1);
    std::string id1 = returnMsg->getString("id");
    delete returnMsg;
    returnMsg = addDummyComponent(url2);
    std::string id2 = returnMsg->getString("id");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_COMPONENTS);

    SocketMessage * reply = ResourceDiscoveryStore::generateRDResponse(&request,resourceDiscoveryStore);

    std::vector<std::string> ids;
    ASSERT_NO_THROW(ids = reply->getArray<std::string>("components"));

    ASSERT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());
    // Make sure we are getting allocated two different IDs
    ASSERT_NE(id1,id2);

    delete reply;
}


TEST_F(SimpleRDS, GetBySchemaValid){
    std::string url1 = "tcp://127.0.0.1:8000";

    SocketMessage * returnMsg = addDummyComponent(url1);
    std::string id1 = returnMsg->getString("id");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_BY_SCHEMA);
    SocketMessage * schema = loadSocketMessage("TestManifests/Endpoint1.json");
    request.addMember("schema",*schema);
    request.addMember("receiveData", true);
    delete schema;


    SocketMessage * reply = ResourceDiscoveryStore::generateRDResponse(&request,resourceDiscoveryStore);
    std::vector<SocketMessage *> endpointReturns = reply->getArray<SocketMessage*>("endpoints");
    ASSERT_GE(endpointReturns.size(), 1);


    delete reply;
    SocketMessage * firstEndpoint = endpointReturns.at(0);
    ASSERT_EQ(firstEndpoint->getString("url"), url1);
    ASSERT_EQ(firstEndpoint->getString("componentId"),id1);

    for (auto e : endpointReturns){
        delete e;
    }
}


TEST_F(SimpleRDS, GetBySchemaInvalid){
    std::string url1 = "tcp://127.0.0.1:8000";

    SocketMessage * returnMsg = addDummyComponent(url1);
    std::string id1 = returnMsg->getString("id");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_BY_SCHEMA);
    // Doesn't match component1
    SocketMessage * schema = loadSocketMessage("TestManifests/Endpoint3.json");
    request.addMember("schema",*schema);
    request.addMember("receiveData", true);
    delete schema;


    SocketMessage * reply = ResourceDiscoveryStore::generateRDResponse(&request,resourceDiscoveryStore);
    std::vector<SocketMessage *> endpointReturns = reply->getArray<SocketMessage*>("endpoints");
    ASSERT_EQ(endpointReturns.size(), 0);


    delete reply;
}