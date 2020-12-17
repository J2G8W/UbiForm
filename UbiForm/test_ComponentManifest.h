#include "gtest/gtest.h"

#include "ComponentManifest.h"

#include <string>

TEST(ComponentManifest, GetNameTest) {
    ComponentManifest testManifest(R"({"name":"TEST1","schemas":{}})");
    std::string expected_output("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);
}

TEST(ComponentManifest, StringifyTest) {
    const char *jsonString = R"({"name":"TEST1","schemas":{}})";
    ComponentManifest testManifest(jsonString);

    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));
}

TEST(ComponentManifest, NoSchema){
    const char *jsonString = R"({"name":"TEST1"})";
    ASSERT_ANY_THROW(new ComponentManifest(jsonString));
}

TEST(ComponentManifest, MalformedManifest){
    const char *jsonString = R"({"name""TEST1")";
    ASSERT_ANY_THROW(new ComponentManifest(jsonString));
}

TEST(ComponentManifest, CreationFromSocketMessage){
    const char *jsonString = R"({"name":"TEST1","schemas":{}})";
    SocketMessage * sm = new SocketMessage(jsonString);

    ComponentManifest testManifest(sm);
    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));

    delete sm;
}

class ManifestExample : public testing::Test{
protected:
    ManifestExample(){
        if (pFile == NULL){
            std::cerr << "Error finding requisite file" << "TestManifests/Component1.json" << std::endl;
        }
        componentManifest = new ComponentManifest(pFile);
    }

    ~ManifestExample(){
        delete componentManifest;
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    ComponentManifest* componentManifest;
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
