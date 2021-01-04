#include "gtest/gtest.h"
#include "SystemSchemas.h"

TEST(SystemSchemas, Init){
    ASSERT_NO_THROW(SystemSchemas ss);
}

TEST(SystemSchemas, ComponentSchema){
    SystemSchemas ss;


    rapidjson::Document * JSON_document = parseFromFile("TestManifests/Component1.json");

    ASSERT_NO_THROW(ss.getSystemSchema(SystemSchemaName::componentManifest).validate(*JSON_document));
}