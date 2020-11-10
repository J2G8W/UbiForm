#include "library.h"

#include "rapidjson/document.h"

#include <iostream>



std::string ComponentManifest::getName() {
    assert(JSON_document.HasMember("name"));
    assert(JSON_document["name"].IsString());

    return JSON_document["name"].GetString();
};