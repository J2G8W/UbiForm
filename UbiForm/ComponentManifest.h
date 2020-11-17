#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H


#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "general_functions.h"


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
        // Arbitrary size of read buffer - only changes efficiency of the inputStream constructor
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
    };

    // No complex delete needed
    ~ComponentManifest()= default;

    // We return a C++ string as this is what we want to be handling inside the program
    std::string getName();
    char* stringify(){return stringifyDocument(JSON_document);};
};


#endif //UBIFORM_COMPONENTMANIFEST_H
