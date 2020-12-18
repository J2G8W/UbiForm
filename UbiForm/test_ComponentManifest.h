#include "gtest/gtest.h"

#include "ComponentManifest.h"

#include <string>

class ComponentManifestBasics :  public testing::Test{
protected:
    SystemSchemas systemSchemas;
};

TEST_F(ComponentManifestBasics, GetNameTest) {
    ComponentManifest testManifest(R"({"name":"TEST1","schemas":{}})", systemSchemas);
    std::string expected_output("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);
}

TEST_F(ComponentManifestBasics, StringifyTest) {
    const char *jsonString = R"({"name":"TEST1","schemas":{}})";
    ComponentManifest testManifest(jsonString, systemSchemas);

    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));
}

TEST_F(ComponentManifestBasics, NoSchema){
    const char *jsonString = R"({"name":"TEST1"})";
    ASSERT_THROW(new ComponentManifest(jsonString, systemSchemas), ValidationError);
}

TEST_F(ComponentManifestBasics, MalformedSchema){
    const char *jsonString = R"({"name":"TEST1","schemas":{"TEST":{"socketType":"NOTHING"}}})";
    ASSERT_THROW(new ComponentManifest(jsonString, systemSchemas), ValidationError);
}

TEST_F(ComponentManifestBasics, MalformedManifest){
    const char *jsonString = R"({"name""TEST1")";
    ASSERT_ANY_THROW(new ComponentManifest(jsonString, systemSchemas));
}

TEST_F(ComponentManifestBasics, CreationFromSocketMessage){
    const char *jsonString = R"({"name":"TEST1","schemas":{}})";
    SocketMessage * sm = new SocketMessage(jsonString);

    ComponentManifest testManifest(sm,systemSchemas);
    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));

    delete sm;
}

class ManifestExample : public testing::Test{
protected:
    ManifestExample() : systemSchemas(){
        if (pFile == NULL){
            std::cerr << "Error finding requisite file" << "TestManifests/Component1.json" << std::endl;
        }
        componentManifest = new ComponentManifest(pFile,systemSchemas);
    }

    ~ManifestExample(){
        delete componentManifest;
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    ComponentManifest* componentManifest;
    SystemSchemas systemSchemas;
};

TEST_F(ManifestExample, ReceiverSchemasTest){
    ASSERT_NO_THROW(componentManifest->getReceiverSchema("pairExample"));

    std::shared_ptr<EndpointSchema> endpointSchema = componentManifest->getReceiverSchema("pairExample");

    SocketMessage socketMessage;
    socketMessage.addMember("temp",50);
    socketMessage.addMember("msg", std::string("HELLO"));

    ASSERT_NO_THROW(endpointSchema->validate(socketMessage));

    socketMessage.addMember("msg", false);
    ASSERT_ANY_THROW(endpointSchema->validate(socketMessage));
}

TEST_F(ManifestExample, SenderSchemasTest){
    ASSERT_NO_THROW(componentManifest->getSenderSchema("pairExample"));

    std::shared_ptr<EndpointSchema> endpointSchema = componentManifest->getSenderSchema("pairExample");

    SocketMessage socketMessage;
    socketMessage.addMember("temp",50);
    socketMessage.addMember("msg", std::string("HELLO"));

    ASSERT_NO_THROW(endpointSchema->validate(socketMessage));

    socketMessage.addMember("msg", false);
    ASSERT_ANY_THROW(endpointSchema->validate(socketMessage));
}
