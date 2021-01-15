#include "gtest/gtest.h"
#include "EndpointSchema.h"
#include "../SocketMessage.h"


class EndpointSchemaSimpleChecks : public ::testing::Test {
protected:
    EndpointSchemaSimpleChecks() : endpointSchema(&schemaDoc.Parse(schemaInput), schemaDoc.GetAllocator()) {
    }

    const char *schemaInput = R"({"properties":{)"
                              R"("temperature":{"type":"number"},)"
                              R"("value":{"type":"string"})"
                              R"(},"required":["value"]})";
    rapidjson::Document schemaDoc;
    EndpointSchema endpointSchema;
};


TEST_F(EndpointSchemaSimpleChecks, ErrorValidationOfMessage) {

    SocketMessage socketMessage;
    // Wrong type
    socketMessage.addMember("temperature", std::string("NOPE"));
    // Correct type
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_THROW(endpointSchema.validate(socketMessage), ValidationError);
}

TEST_F(EndpointSchemaSimpleChecks, CorrectValidationOfMessage) {
    SocketMessage socketMessage;
    // Both are correct types
    socketMessage.addMember("temperature", 42);
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_NO_THROW(endpointSchema.validate(socketMessage));
}

TEST_F(EndpointSchemaSimpleChecks, GetSchema) {
    SocketMessage *schemaRep = endpointSchema.getSchemaObject();
    ASSERT_EQ(schemaRep->stringify(), std::string(schemaInput));
    delete schemaRep;
}

TEST_F(EndpointSchemaSimpleChecks, GetType) {
    ASSERT_EQ(endpointSchema.getValueType("temperature"), ValueType::Number);
    ASSERT_EQ(endpointSchema.getValueType("value"), ValueType::String);
    ASSERT_THROW(endpointSchema.getValueType("NOT A FIELD"), AccessError);

    // Make sure the schema hasn't been changed
    SocketMessage *schemaRep = endpointSchema.getSchemaObject();
    ASSERT_EQ(schemaRep->stringify(), std::string(schemaInput));
    delete schemaRep;
}

TEST_F(EndpointSchemaSimpleChecks, SimpleUpdate) {
    rapidjson::Document *JSON_document = parseFromFile("TestManifests/Endpoint1.json");

    endpointSchema.completeUpdate(*JSON_document);
    ASSERT_EQ(endpointSchema.getValueType("temp"), ValueType::Number);
    ASSERT_THROW(endpointSchema.getValueType("temperature"), AccessError);

    // The base document has been changed
    ASSERT_NE(stringifyValue(schemaDoc), schemaInput);

    delete JSON_document;
}

TEST_F(EndpointSchemaSimpleChecks, GetNames) {
    std::vector<std::string> requiredAttributes = endpointSchema.getRequired();
    std::vector<std::string> correctRequiredAttributes{"value"};
    ASSERT_EQ(requiredAttributes, correctRequiredAttributes);

    std::vector<std::string> attributes = endpointSchema.getAllProperties();
    std::vector<std::string> correctPropertyAttributes{"temperature", "value"};
    ASSERT_EQ(attributes, correctPropertyAttributes);
}

TEST_F(EndpointSchemaSimpleChecks, AddAttribute) {
    endpointSchema.addProperty("msg", ValueType::String);
    ASSERT_EQ(endpointSchema.getValueType("msg"), ValueType::String);
    endpointSchema.addProperty("msg", ValueType::Number);
    ASSERT_EQ(endpointSchema.getValueType("msg"), ValueType::Number);

    SocketMessage sm;
    sm.addMember("msg", 20);
    sm.addMember("value", "HELLO");
    sm.addMember("temperature", 1000);

    ASSERT_NO_THROW(endpointSchema.validate(sm));

    sm.addMember("msg", "HELLO");

    ASSERT_THROW(endpointSchema.validate(sm), ValidationError);
}

TEST_F(EndpointSchemaSimpleChecks, RemoveRequired) {
    endpointSchema.removeRequired("value");
    SocketMessage sm;
    ASSERT_NO_THROW(endpointSchema.validate(sm));

    endpointSchema.addRequired("temperature");
    ASSERT_THROW(endpointSchema.validate(sm), ValidationError);

    sm.addMember("temperature", 0);
    ASSERT_NO_THROW(endpointSchema.validate(sm));
}

TEST_F(EndpointSchemaSimpleChecks, ArrayAddition) {
    endpointSchema.setArrayType("msgs", ValueType::String);
    endpointSchema.setArrayType("msgs", ValueType::Number);

    SocketMessage sm;
    sm.addMember("value", "HELLO");
    std::vector<int> msgs{1, 2, 3, 4};
    sm.addMember("msgs", msgs);
    ASSERT_NO_THROW(endpointSchema.validate(sm));

    std::vector<bool> falseMsgs{false, false, false};
    sm.addMember("msgs", falseMsgs);
    ASSERT_THROW(endpointSchema.validate(sm), ValidationError);
}

TEST_F(EndpointSchemaSimpleChecks, SubObjects) {
    EndpointSchema subEndpoint;
    subEndpoint.addProperty("TEST", ValueType::Number);
    subEndpoint.addRequired("TEST");
    endpointSchema.setSubObject("sub", subEndpoint);

    SocketMessage sm;
    sm.addMember("value", "HELLO");
    ASSERT_NO_THROW(endpointSchema.validate(sm));
    SocketMessage subSM;
    sm.addMember("sub", subSM);
    ASSERT_THROW(endpointSchema.validate(sm), ValidationError);
}

TEST_F(EndpointSchemaSimpleChecks, SubArrayObjects) {
    EndpointSchema subEndpoint;
    subEndpoint.addProperty("TEST", ValueType::Number);
    subEndpoint.addRequired("TEST");
    endpointSchema.setArrayObject("sub", subEndpoint);

    SocketMessage sm;
    sm.addMember("value", "HELLO");

    SocketMessage *subSm = new SocketMessage;
    subSm->addMember("TEST", 10);

    std::vector<SocketMessage *> arrayS = {subSm, subSm};
    sm.addMember("sub", arrayS);
    delete subSm;

    ASSERT_NO_THROW(endpointSchema.validate(sm));

    SocketMessage *wrongSub = new SocketMessage;
    wrongSub->addMember("TEST", "NOT ALLOWED");
    arrayS = {wrongSub, wrongSub};
    sm.addMember("sub", arrayS);
    delete wrongSub;

    ASSERT_THROW(endpointSchema.validate(sm), ValidationError);


}