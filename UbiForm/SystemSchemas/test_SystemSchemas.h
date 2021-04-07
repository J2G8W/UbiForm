#include "gtest/gtest.h"
#include "SystemSchemas.h"

TEST(SystemSchemas, Init) {
    ASSERT_NO_THROW(SystemSchemas ss);
}

TEST(SystemSchemas, ComponentSchema) {
    SystemSchemas ss;


    rapidjson::Document *JSON_document = parseFromFile("TestManifests/Component1.json");

    ASSERT_NO_THROW(ss.getSystemSchema(SystemSchemaName::componentManifest).validate(*JSON_document));
    // Make sure we haven't got blank schemas coming in
    ASSERT_THROW(ss.getSystemSchema(SystemSchemaName::additionRequest).validate(*JSON_document), ValidationError);
    delete JSON_document;
}