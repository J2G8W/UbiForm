#ifndef UBIFORM_GENERICSCHEMA_H
#define UBIFORM_GENERICSCHEMA_H

#include "SocketMessage.h"
#include "endpoints/EndpointSchema.h"

class GenericSchema {
private:
    EndpointSchema * es;
    rapidjson::Document  document;

    static rapidjson::Document InitiateFromFile(FILE *jsonFP){
        rapidjson::Document JSON_document;
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
        return JSON_document;
    }

public:
    explicit GenericSchema(FILE * jsonFP) : document(InitiateFromFile(jsonFP)){
        es = new EndpointSchema(&document, document.GetAllocator());
    }
    void validate(const SocketMessage &messageToValidate){
        es->validate(messageToValidate);
    }
    void validate(const rapidjson::Value &doc){
        es->validate(doc);
    }
};


#endif //UBIFORM_GENERICSCHEMA_H
