#include "gtest/gtest.h"
#include "EndpointSchema.h"
#include "SocketMessage.h"


class EndpointSchemaSimpleChecks : public ::testing::Test{
protected:
    EndpointSchemaSimpleChecks(): endpointSchema(&schemaDoc.Parse(schemaInput), schemaDoc.GetAllocator()){
    }

    const char *schemaInput = R"({"properties":{)"
                                R"("temperature":{"type":"number"},)"
                                R"("value":{"type":"string"})"
                                R"(}})";
    rapidjson::Document schemaDoc;
    EndpointSchema endpointSchema;
};


TEST_F(EndpointSchemaSimpleChecks, ErrorValidationOfMessage){

    SocketMessage socketMessage;
    // Wrong type
    socketMessage.addMember("temperature",std::string("NOPE"));
    // Correct type
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_THROW(endpointSchema.validate(socketMessage), ValidationError);
}

TEST_F(EndpointSchemaSimpleChecks, CorrectValidationOfMessage){
    SocketMessage socketMessage;
    // Both are correct types
    socketMessage.addMember("temperature", 42);
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_NO_THROW(endpointSchema.validate(socketMessage));
}

TEST_F(EndpointSchemaSimpleChecks, GetSchema){
    SocketMessage * schemaRep = endpointSchema.getSchemaObject();
    ASSERT_EQ(schemaRep->stringify(), std::string(schemaInput));
}

TEST_F(EndpointSchemaSimpleChecks, GetType){
    ASSERT_EQ(endpointSchema.getValueType("temperature"), ValueType::Number);
    ASSERT_EQ(endpointSchema.getValueType("value"), ValueType::String);
    ASSERT_THROW(endpointSchema.getValueType("NOT A FIELD"), AccessError);

    // Make sure the schema hasn't been changed
    SocketMessage * schemaRep = endpointSchema.getSchemaObject();
    ASSERT_EQ(schemaRep->stringify(), std::string(schemaInput));
}

TEST_F(EndpointSchemaSimpleChecks, SimpleUpdate){
    rapidjson::Document * JSON_document = parseFromFile("TestManifests/Endpoint1.json");

    endpointSchema.updateSchema(*JSON_document);
    ASSERT_EQ(endpointSchema.getValueType("temp"),ValueType::Number);
    ASSERT_THROW(endpointSchema.getValueType("temperature"), AccessError);

    // The base document has been changed
    ASSERT_NE(stringifyDocument(schemaDoc), schemaInput);
}