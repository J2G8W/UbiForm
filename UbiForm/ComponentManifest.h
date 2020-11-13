#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H


#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

class ComponentManifest {
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

    const char* stringify(){
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        JSON_document.Accept(writer);
        return buffer.GetString();
    }
};


#endif //UBIFORM_COMPONENTMANIFEST_H
