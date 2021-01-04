#include "gtest/gtest.h"
#include "SystemSchemas.h"

TEST(SystemSchemas, Init){
    ASSERT_NO_THROW(SystemSchemas ss);
}

TEST(SystemSchemas, ComponentSchema){
    SystemSchemas ss;
    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    if (pFile == nullptr){
        std::cout << "Error opening file - " << std::endl;
        throw std::system_error();
    }

    rapidjson::Document JSON_document;
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(pFile, readBuffer, sizeof(readBuffer));
    JSON_document.ParseStream(inputStream);

    fclose(pFile);

    ASSERT_NO_THROW(ss.getSystemSchema(SystemSchemaName::componentManifest).validate(JSON_document));
}