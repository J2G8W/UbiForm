#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "../SocketMessage.h"

class EndpointSchema {
private:
    rapidjson::SchemaDocument schema;

public:
    explicit EndpointSchema(rapidjson::Value &doc) : schema(doc) { }
    explicit EndpointSchema(FILE *jsonFP) : schema(InitiateFromFile(jsonFP)){
    }

    // TODO - make this less nasty
    static rapidjson::Document InitiateFromFile(FILE *jsonFP){
        rapidjson::Document JSON_document;
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
        return JSON_document;
    }

    void validate(const SocketMessage &messageToValidate);
};


#endif //UBIFORM_ENDPOINTSCHEMA_H
