#include "ComponentManifest.h"
#include <iostream>
#include <cstring>

// Return the name of the Component
std::string ComponentManifest::getName() {
    assert(JSON_document.HasMember("name"));
    assert(JSON_document["name"].IsString());

    return JSON_document["name"].GetString();
}

// Return the string of the manifest (for sending on)
// TODO - optimise this for speed
char* ComponentManifest::stringify(){
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    JSON_document.Accept(writer);

    // We copy the string from the buffer to our return string so that it is not squashed when we return
    int stringLength = buffer.GetLength();
    char* jsonReturnString = new char[stringLength];
    strncpy(jsonReturnString, buffer.GetString(), stringLength);

    return jsonReturnString;
}