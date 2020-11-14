#include "ComponentManifest.h"
#include <iostream>

std::string ComponentManifest::getName() {
    assert(JSON_document.HasMember("name"));
    assert(JSON_document["name"].IsString());

    return JSON_document["name"].GetString();
}

rapidjson::StringBuffer ComponentManifest::stringify(){
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    JSON_document.Accept(writer);

    return buffer;
}