#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "UtilityFunctions.h"



// TODO - optimise this for speed
std::string stringifyValue(rapidjson::Value &JSON_document) {
    rapidjson::StringBuffer buffer;

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    JSON_document.Accept(writer);

    // We copy the string from the buffer to our return string so that it is not squashed when we return
    std::string jsonReturnString(buffer.GetString());

    return jsonReturnString;
}


bool compareSchemaObjects(rapidjson::Value &schema1, rapidjson::Value &schema2) {
    if (!schema1.IsObject() || !schema2.IsObject()){ return false;}
    if (! (schema1.HasMember("type") && schema1["type"].IsString() && strncmp(schema1["type"].GetString(),"object",6) == 0)){
        return false;
    }
    if (! (schema2.HasMember("type") && schema2["type"].IsString() && strncmp(schema2["type"].GetString(),"object",6) == 0)){
        return false;
    }
    if (!(schema1.HasMember("properties") && schema1["properties"].IsObject()
        && schema2.HasMember("properties") && schema2["properties"].IsObject())){
        return false;
    }
    if (schema1["properties"].Size() != schema2["properties"].Size()){return false;}

    auto properties1 = schema1["properties"].GetObject();
    auto properties2 = schema2["properties"].GetObject();
    for (auto &v : properties1){
        const char * name = v.name.GetString();
        if (!(properties2.HasMember(name))) {return false;}

        if (!(v.value.IsObject() && properties2[name].IsObject())){return false;}

        if (!(v.value.GetObject().HasMember("type") && v.value.GetObject()["type"].IsString() &&
            properties2[name].GetObject().HasMember("type") && properties2[name].GetObject()["type"].IsString())){
            return false;
        }
        const char * v1Type = v.value.GetObject()["type"].GetString();
        const char * v2Type = properties2[name].GetObject()["type"].GetString();
        if (strncmp(v1Type,"object",6) == 0){
            if (! compareSchemaObjects(v.value,properties2[name])){ return false;}
        }else if (strncmp(v1Type, "array",5) == 0){
            if (! compareSchemaArrays(v.value,properties2[name])){return false;}
        }else{
            if (strncmp(v1Type,v2Type, strlen(v1Type)) != 0){return false;}
        }
    }

    if (schema1.HasMember("required") && schema2.HasMember("required")){
        if (!(schema1["required"].IsArray() && schema2["required"].IsArray())){return false;}
        auto required1 = schema1["required"].GetArray();
        auto required2 = schema2["required"].GetArray();
        if (required1.Size() != required2.Size()){return  false;}
        // TODO

    }else if(schema1.HasMember("required") || schema2.HasMember("required")){return false;}

    return true;
}



bool compareSchemaArrays(rapidjson::Value &schema1, rapidjson::Value &schema2) {
    // We assume we have been given Values which have type = array

    // Neither thing has an "items"
    if (!(schema1.HasMember("items") || schema1.HasMember("items"))){return true;}

    if (!(schema1.HasMember("items") && schema1["items"].IsObject() && schema2.HasMember("items") && schema2["items"].IsObject())){
        return false;
    }

    auto items1 = schema1["items"].GetObject();
    auto items2 = schema2["items"].GetObject();

    if (!(items1.HasMember("type") && items1["type"].IsString() && items2.HasMember("type") && items2["type"].IsString())){
        return false;
    }
    if (strncmp(items1["type"].GetString(),items2["type"].GetString(), items1["type"].GetStringLength()) != 0){
        return false;
    }

    if (strncmp(items1["type"].GetString(),"object", 6) == 0){
        return compareSchemaObjects(schema1["items"],schema2["items"]);
    }

    return true;
}

rapidjson::Document *parseFromFile(const char *address) {
    FILE* pFile = fopen(address, "r");
    if (pFile == nullptr){
        std::string errorMsg = "Error opening file - " + std::string(address);
        throw AccessError(errorMsg);
    }

    auto * JSON_document = new rapidjson::Document ;
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(pFile, readBuffer, sizeof(readBuffer));
    JSON_document->ParseStream(inputStream);
    fclose(pFile);
    return JSON_document;
}
