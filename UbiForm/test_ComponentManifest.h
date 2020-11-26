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

class ManifestExample : public testing::Test{
protected:
    ManifestExample(){
        if (pFile == NULL){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        componentManifest = new ComponentManifest(pFile);
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    ComponentManifest* componentManifest;
};

TEST_F(ManifestExample, ReceiverSchemasTest){
    ASSERT_NO_THROW(componentManifest->getReceiverSchema("v1"));

    EndpointSchema* endpointSchema = componentManifest->getReceiverSchema("v1");

    SocketMessage socketMessage;
    socketMessage.addMember("temp",50);
    socketMessage.addMember("msg", std::string("HELLO"));

    ASSERT_NO_THROW(endpointSchema->validate(socketMessage));

    socketMessage.addMember("msg", false);
    ASSERT_ANY_THROW(endpointSchema->validate(socketMessage));
}

TEST_F(ManifestExample, SenderSchemasTest){
    ASSERT_NO_THROW(componentManifest->getSenderSchema("v1"));

    EndpointSchema* endpointSchema = componentManifest->getSenderSchema("v1");

    SocketMessage socketMessage;
    socketMessage.addMember("temp",50);
    socketMessage.addMember("msg", std::string("HELLO"));

    ASSERT_NO_THROW(endpointSchema->validate(socketMessage));

    socketMessage.addMember("msg", false);
    ASSERT_ANY_THROW(endpointSchema->validate(socketMessage));
}


// TODO - add testing for the private methods