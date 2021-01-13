#include "ResourceDiscoveryStore.h"

#include <algorithm>
#include <fstream>

class SimpleRDS : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleRDS(): ss(), resourceDiscoveryStore(ss){
        if (pFile == nullptr){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        exampleManifest = new ComponentManifest(pFile,ss);
    }

    ~SimpleRDS(){
        delete exampleManifest;
    }

    SocketMessage *addDummyComponent() {
        SocketMessage sm;
        sm.addMember("request", ADDITION);

        auto manifest = exampleManifest->getSocketMessageCopy();
        sm.addMoveObject("manifest", std::move(manifest));


        SocketMessage * returnMsg;
        returnMsg = resourceDiscoveryStore.generateRDResponse(&sm);

        return returnMsg;
    }

    SocketMessage * loadSocketMessage(const std::string& location){
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
    SystemSchemas ss;
};


TEST_F(SimpleRDS,AdditionOfComponent){
    SocketMessage * sm = new SocketMessage;
    sm->addMember("request", ADDITION);

    auto manifest = exampleManifest->getSocketMessageCopy();
    sm->addMoveObject("manifest", std::move(manifest));


    SocketMessage * returnMsg = nullptr;
    // In this assertion we know that we have got back a valid message
    ASSERT_NO_THROW( returnMsg = resourceDiscoveryStore.generateRDResponse(sm));


    delete returnMsg;
    delete sm;
}


TEST_F(SimpleRDS,GetManifestById){
    SocketMessage * returnMsg = addDummyComponent();

    // Assert that we have a returned ID
    std::string returnID = returnMsg->getString("newID");

    // We delete the returned message because it is safe to do so
    delete returnMsg;


    SocketMessage idRequest;
    idRequest.addMember("request",REQUEST_BY_ID);
    idRequest.addMember("id",returnID);

    returnMsg = resourceDiscoveryStore.generateRDResponse(&idRequest);

    ASSERT_NO_THROW(returnMsg->isNull("component"));
    ASSERT_FALSE(returnMsg->isNull("component"));


    std::unique_ptr<SocketMessage> componentObject;
    ASSERT_NO_THROW(componentObject = returnMsg->getCopyObject("component"));

    ComponentRepresentation componentRepresentation(componentObject.get(),ss);

    delete returnMsg;
}

TEST_F(SimpleRDS, GetComponentIds){

    SocketMessage * returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");
    delete returnMsg;
    returnMsg = addDummyComponent();
    std::string id2 = returnMsg->getString("newID");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_COMPONENTS);

    SocketMessage * reply = resourceDiscoveryStore.generateRDResponse(&request);

    std::vector<std::string> ids;
    ASSERT_NO_THROW(ids = reply->getArray<std::string>("components"));

    ASSERT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());
    // Make sure we are getting allocated two different IDs
    ASSERT_NE(id1,id2);

    delete reply;
}


TEST_F(SimpleRDS, GetBySchemaValid){

    SocketMessage * returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_BY_SCHEMA);
    auto schema = std::unique_ptr<SocketMessage>(loadSocketMessage("TestManifests/Endpoint1.json"));
    request.addMoveObject("schema", std::move(schema));
    request.addMember("dataReceiverEndpoint", true);
    request.addMoveObject("specialProperties", std::make_unique<SocketMessage>());



    SocketMessage * reply = resourceDiscoveryStore.generateRDResponse(&request);
    std::vector<std::unique_ptr<SocketMessage>> endpointReturns = reply->getArray<std::unique_ptr<SocketMessage>>("endpoints");
    ASSERT_GE(endpointReturns.size(), 1);


    delete reply;
    ASSERT_EQ(endpointReturns.at(0)->getString("componentId"),id1);

    SocketMessage newRequest;
    newRequest.addMember("request",REQUEST_BY_SCHEMA);
    schema = std::unique_ptr<SocketMessage>(loadSocketMessage("TestManifests/Endpoint1.json"));
    newRequest.addMoveObject("schema", std::move(schema));
    newRequest.addMember("dataReceiverEndpoint", true);


    auto errorSpecialProperties = std::make_unique<SocketMessage>();
    errorSpecialProperties->addMember("name","NOT FOUND");
    newRequest.addMoveObject("specialProperties", std::move(errorSpecialProperties));

    reply = resourceDiscoveryStore.generateRDResponse(&newRequest);
    endpointReturns = reply->getArray<std::unique_ptr<SocketMessage>>("endpoints");
    ASSERT_EQ(endpointReturns.size(), 0);
    delete reply;
}


TEST_F(SimpleRDS, GetBySchemaInvalid){
    std::string url1 = "tcp://127.0.0.1:8000";

    SocketMessage * returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");
    delete returnMsg;

    SocketMessage request;
    request.addMember("request",REQUEST_BY_SCHEMA);
    // Doesn't match component1
    SocketMessage * schema = loadSocketMessage("TestManifests/Endpoint3.json");
    request.addMember("schema",*schema);
    request.addMember("dataReceiverEndpoint", true);
    request.addMoveObject("specialProperties", std::make_unique<SocketMessage>());
    delete schema;


    SocketMessage * reply = resourceDiscoveryStore.generateRDResponse(&request);
    auto endpointReturns = reply->getArray<std::unique_ptr<SocketMessage>>("endpoints");
    ASSERT_EQ(endpointReturns.size(), 0);

    delete reply;
}

TEST_F(SimpleRDS, NoRequestField){
    SocketMessage request;
    ASSERT_THROW(resourceDiscoveryStore.generateRDResponse(&request), ValidationError);
}

TEST_F(SimpleRDS, BrokenRequest){
    SocketMessage request;
    request.addMember("request",ADDITION);

    ASSERT_THROW(resourceDiscoveryStore.generateRDResponse(&request), ValidationError);
}