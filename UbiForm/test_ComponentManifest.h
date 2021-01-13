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


TEST_F(ComponentManifestBasics, EmptyBase){
    ComponentManifest testManifest(systemSchemas);
    testManifest.setName("TEST");
    ASSERT_EQ(testManifest.getName(), "TEST");
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
    auto * sm = new SocketMessage(jsonString);

    ComponentManifest testManifest(sm,systemSchemas);
    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));

    delete sm;
}

TEST_F(ComponentManifestBasics, AdditionalInfo){
    ComponentManifest testManifest(systemSchemas);
    testManifest.setProperty("public_key","HELLO");
    ASSERT_NO_THROW(testManifest.getProperty("public_key"));
    ASSERT_EQ(testManifest.getProperty("public_key"), "HELLO");

    ASSERT_THROW(testManifest.getProperty("NOT A PROPERTY"),AccessError);

    ASSERT_NO_THROW(ComponentManifest copiedManifest(testManifest.getSocketMessageCopy().get(),systemSchemas));

    ASSERT_TRUE(testManifest.hasProperty("public_key"));
    testManifest.removeProperty("public_key");
    ASSERT_FALSE(testManifest.hasProperty("public_key"));

}

TEST_F(ComponentManifestBasics, CreationFromFile){
    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    if (pFile == nullptr){
        std::cerr << "Error finding requisite file" << "TestManifests/Component1.json" << std::endl;
    }
    ComponentManifest cm(pFile, systemSchemas);
    fseek(pFile,SEEK_SET,0);

    ComponentManifest cm2(systemSchemas);
    cm2.setManifest(pFile);

    ASSERT_EQ(cm.stringify(),cm2.stringify());
}

class ManifestExample : public testing::Test{
protected:
    ManifestExample() : systemSchemas(){
        if (pFile == nullptr){
            std::cerr << "Error finding requisite file" << "TestManifests/Component1.json" << std::endl;
        }
        componentManifest = new ComponentManifest(pFile,systemSchemas);
    }

    ~ManifestExample() override{
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


TEST_F(ManifestExample, AddPairSchema){
    std::shared_ptr<EndpointSchema> sendSchema = std::make_shared<EndpointSchema>();
    std::shared_ptr<EndpointSchema> receiveSchema = std::make_shared<EndpointSchema>();
    receiveSchema->addProperty("TEST",ValueType::Number);
    receiveSchema->addRequired("TEST");
    componentManifest->addEndpoint(SocketType::Pair, "pairExample", receiveSchema, sendSchema);

    SocketMessage sm;
    sm.addMember("TEST",42);
    ASSERT_NO_THROW(componentManifest->getReceiverSchema("pairExample")->validate(sm));

    SocketMessage sm2;
    sm.addMember("TEST","HELLO");
    ASSERT_THROW(componentManifest->getReceiverSchema("pairExample")->validate(sm),ValidationError);

    SocketMessage* schemaRep = componentManifest->getSchemaObject("pairExample",true);
    auto requiredArray = schemaRep->getArray<std::string>("required");

    ASSERT_EQ(requiredArray.size(), 1);
    ASSERT_EQ(requiredArray.at(0), "TEST");
    delete schemaRep;
}

TEST_F(ManifestExample, AddSubscriberSchema){
    std::shared_ptr<EndpointSchema> receiveSchema = std::make_shared<EndpointSchema>();
    receiveSchema->addProperty("TEST",ValueType::Number);
    receiveSchema->addRequired("TEST");
    componentManifest->addEndpoint(SocketType::Subscriber, "subExample", receiveSchema, nullptr);

    SocketMessage sm;
    ASSERT_THROW(componentManifest->getReceiverSchema("subExample")->validate(sm), ValidationError);
}