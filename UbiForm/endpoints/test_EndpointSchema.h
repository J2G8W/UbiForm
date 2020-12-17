#include "gtest/gtest.h"
#include "EndpointSchema.h"
#include "../SocketMessage.h"


class SchemaTest : public ::testing::Test{
protected:
    SchemaTest():endpointSchema(schemaDoc.Parse(schemaInput)){
    }

    const char *schemaInput = R"({"properties":{)"
                                R"("temperature":{"type": "number"},)"
                                R"("value":{"type": "string"})"
                                R"(}})";
    rapidjson::Document schemaDoc;
    EndpointSchema endpointSchema;
};


TEST_F(SchemaTest, ErrorValidationOfMessage){

    SocketMessage socketMessage;
    // Wrong type
    socketMessage.addMember("temperature",std::string("NOPE"));
    // Correct type
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_THROW(endpointSchema.validate(socketMessage), ValidationError);
}

TEST_F(SchemaTest, CorrectValidationOfMessage){
    SocketMessage socketMessage;
    // Both are correct types
    socketMessage.addMember("temperature", 42);
    socketMessage.addMember("value", std::string("WORLD"));

    ASSERT_NO_THROW(endpointSchema.validate(socketMessage));
}

