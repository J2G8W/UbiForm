#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H


#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include "rapidjson/schema.h"
#include "rapidjson/error/en.h"

#include "general_functions.h"
#include "SocketMessage.h"


class ComponentManifest {
private:
    rapidjson::Document JSON_document;
    rapidjson::SchemaDocument *schema;

    void checkParse();


public:
    // Accept JSON input as string
    explicit ComponentManifest(const char *jsonString);

    // Accept JSON input as a FILE pointer
    // This is used rather than istreams as we get better performance for rapidjson
    explicit ComponentManifest(FILE *jsonFP);

    // Delete our schema object
    ~ComponentManifest(){
        delete schema;
    };

    // We return C++ strings such that memory management is simpler
    std::string getName();

    std::string stringify() { return stringifyDocument(JSON_document); };

    // We check the given message against our schema
    void validate(const SocketMessage &messageToValidate);
};


#endif //UBIFORM_COMPONENTMANIFEST_H
