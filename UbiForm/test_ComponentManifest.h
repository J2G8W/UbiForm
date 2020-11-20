#include "gtest/gtest.h"

#include "ComponentManifest.h"

#include <string>

TEST(ComponentManifest, GetNameTest) {
    ComponentManifest testManifest(R"({"name":"TEST1","schema":{}})");
    std::string expected_output("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);
}

TEST(ComponentManifest, StringifyTest) {
    const char *jsonString = R"({"name":"TEST1","schema":{}})";
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

class ManifestTest : public ::testing::Test{
protected:
    ManifestTest(): manifest(manifestInput){
    }

    const char *manifestInput = R"({"name":"TEST1",)"
                                R"("schema":{"properties":{)"
                                    R"("temperature":{"type": "number"},)"
                                    R"("value":{"type": "string"})"
                                R"(}}})";
    ComponentManifest manifest;
};

TEST_F(ManifestTest, ErrorValidationOfMessage){

    SocketMessage socketMessage;
    // Wrong type
    socketMessage.addMember("temperature",std::string("NOPE"));
    // Correct type
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_ANY_THROW(manifest.validate(socketMessage));
}

TEST_F(ManifestTest, CorrectValidationOfMessage){
    SocketMessage socketMessage;
    // Both are correct types
    socketMessage.addMember("temperature", 42);
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_NO_THROW(manifest.validate(socketMessage));
}