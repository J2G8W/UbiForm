#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryStore.h"

#include <algorithm>
#include <fstream>

class SimpleRDS : public testing::Test {
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleRDS() : ss(), resourceDiscoveryStore(ss) {
        if (pFile == nullptr) {
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        exampleManifest = new ComponentManifest(pFile, ss);
    }

    ~SimpleRDS() {
        delete exampleManifest;
    }

    std::unique_ptr<EndpointMessage> addDummyComponent() {
        EndpointMessage sm;
        sm.addMember("request", RESOURCE_DISCOVERY_ADD_COMPONENT);

        auto manifest = exampleManifest->getSocketMessageCopy();
        sm.addMoveObject("manifest", std::move(manifest));

        std::unique_ptr<EndpointMessage> returnMsg = resourceDiscoveryStore.generateRDResponse(&sm);

        return returnMsg;
    }

    std::unique_ptr<EndpointMessage> loadSocketMessage(const std::string &location) {
        try {
            std::ifstream in(location);
            std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            auto socketMessage = std::make_unique<EndpointMessage>(contents.c_str());
            return socketMessage;
        } catch (const std::ifstream::failure &e) {
            std::cerr << "ERROR OPENING FILE: " << location << std::endl;
            throw;
        }
    }

    FILE *pFile = fopen("TestManifests/Component1.json", "r");
    ResourceDiscoveryStore resourceDiscoveryStore;
    ComponentManifest *exampleManifest;
    SystemSchemas ss;
};


TEST_F(SimpleRDS, AdditionOfComponent) {
    EndpointMessage sm;
    sm.addMember("request", RESOURCE_DISCOVERY_ADD_COMPONENT);

    std::unique_ptr<EndpointMessage> manifest = exampleManifest->getSocketMessageCopy();
    sm.addMoveObject("manifest", std::move(manifest));



    // In this assertion we know that we have got back a valid message
    ASSERT_NO_THROW(std::unique_ptr<EndpointMessage> returnMsg = resourceDiscoveryStore.generateRDResponse(&sm));
}


TEST_F(SimpleRDS, GetManifestById) {
    std::unique_ptr<EndpointMessage> returnMsg = addDummyComponent();

    // Assert that we have a returned ID
    std::string returnID = returnMsg->getString("newID");


    EndpointMessage idRequest;
    idRequest.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_ID);
    idRequest.addMember("id", returnID);

    returnMsg = resourceDiscoveryStore.generateRDResponse(&idRequest);

    ASSERT_NO_THROW(returnMsg->isNull("component"));
    ASSERT_FALSE(returnMsg->isNull("component"));


    std::unique_ptr<EndpointMessage> componentObject;
    ASSERT_NO_THROW(componentObject = returnMsg->getCopyObject("component"));

    ComponentRepresentation componentRepresentation(componentObject.get(), ss);
}

TEST_F(SimpleRDS, GetComponentIds) {

    std::unique_ptr<EndpointMessage> returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");
    returnMsg = addDummyComponent();
    std::string id2 = returnMsg->getString("newID");


    EndpointMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_COMPONENTS);

    std::unique_ptr<EndpointMessage> reply = resourceDiscoveryStore.generateRDResponse(&request);

    std::vector<std::string> ids;
    ASSERT_NO_THROW(ids = reply->getArray<std::string>("components"));

    ASSERT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    ASSERT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());
    // Make sure we are getting allocated two different IDs
    ASSERT_NE(id1, id2);
}


TEST_F(SimpleRDS, GetBySchemaValid) {

    std::unique_ptr<EndpointMessage> returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");

    EndpointMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA);
    std::unique_ptr<EndpointMessage> schema = loadSocketMessage("TestManifests/Endpoint1.json");
    request.addMoveObject("schema", std::move(schema));
    request.addMember("dataReceiverEndpoint", true);
    request.addMoveObject("specialProperties", std::make_unique<EndpointMessage>());


    std::unique_ptr<EndpointMessage> reply = resourceDiscoveryStore.generateRDResponse(&request);
    std::vector<std::unique_ptr<EndpointMessage>> endpointReturns = reply->getArray<std::unique_ptr<EndpointMessage>>(
            "endpoints");
    ASSERT_GE(endpointReturns.size(), 1);

    ASSERT_EQ(endpointReturns.at(0)->getString("componentId"), id1);

    EndpointMessage newRequest;
    newRequest.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA);
    schema = loadSocketMessage("TestManifests/Endpoint1.json");
    newRequest.addMoveObject("schema", std::move(schema));
    newRequest.addMember("dataReceiverEndpoint", true);


    std::unique_ptr<EndpointMessage> errorSpecialProperties = std::make_unique<EndpointMessage>();
    errorSpecialProperties->addMember("name", "NOT FOUND");
    newRequest.addMoveObject("specialProperties", std::move(errorSpecialProperties));

    reply = resourceDiscoveryStore.generateRDResponse(&newRequest);
    endpointReturns = reply->getArray<std::unique_ptr<EndpointMessage>>("endpoints");
    ASSERT_EQ(endpointReturns.size(), 0);
}


TEST_F(SimpleRDS, GetBySchemaInvalid) {
    std::string url1 = "tcp://127.0.0.1:8000";

    std::unique_ptr<EndpointMessage> returnMsg = addDummyComponent();
    std::string id1 = returnMsg->getString("newID");

    EndpointMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA);
    // Doesn't match component1
    std::unique_ptr<EndpointMessage> schema = loadSocketMessage("TestManifests/Endpoint3.json");
    request.addMember("schema", *schema);
    request.addMember("dataReceiverEndpoint", true);
    request.addMoveObject("specialProperties", std::make_unique<EndpointMessage>());


    std::unique_ptr<EndpointMessage> reply = resourceDiscoveryStore.generateRDResponse(&request);
    auto endpointReturns = reply->getArray<std::unique_ptr<EndpointMessage>>("endpoints");
    ASSERT_EQ(endpointReturns.size(), 0);

}

TEST_F(SimpleRDS, NoRequestField) {
    EndpointMessage request;
    ASSERT_THROW(resourceDiscoveryStore.generateRDResponse(&request), ValidationError);
}

TEST_F(SimpleRDS, BrokenRequest) {
    EndpointMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_ADD_COMPONENT);

    ASSERT_THROW(resourceDiscoveryStore.generateRDResponse(&request), ValidationError);
}