#ifndef UBIFORM_LIBRARY_H
#define UBIFORM_LIBRARY_H

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

class ComponentManifest{
private:
    rapidjson::Document JSON_document;

public:
    // Accept JSON input as string
    explicit ComponentManifest(const char* jsonString){
        rapidjson::StringStream stream(jsonString);
        JSON_document.ParseStream(stream);
    };
    // Accept JSON input as a FILE pointer
    explicit ComponentManifest(FILE* jsonFP){
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
    };
    std::string getName();

};

class Component{
private:
    ComponentManifest manifest;



};

#endif //UBIFORM_LIBRARY_H
