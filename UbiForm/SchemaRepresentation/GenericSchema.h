#ifndef UBIFORM_GENERICSCHEMA_H
#define UBIFORM_GENERICSCHEMA_H

#include "../../include/UbiForm/EndpointMessage.h"
#include "../../include/UbiForm/SchemaRepresentation/EndpointSchema.h"

/**
 * We have wrapped an EndpointSchema object as these are to represent our SystemSchemas, and we don't need them to be changeable
 */
class GenericSchema {
private:
    std::shared_ptr<EndpointSchema> es;
    rapidjson::Document document;

    static rapidjson::Document InitiateFromFile(FILE *jsonFP) {
        rapidjson::Document JSON_document;
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
        return JSON_document;
    }

public:
    /**
     * Creates a schema from file
     * @param jsonFP
     */
    explicit GenericSchema(FILE *jsonFP) : document(InitiateFromFile(jsonFP)) {
        es = std::make_shared<EndpointSchema>(&document, document.GetAllocator());
    }

    GenericSchema() : document() {
        es = std::make_shared<EndpointSchema>();
    }

    void validate(const EndpointMessage &messageToValidate) {
        es->validate(messageToValidate);
    }

    void validate(const rapidjson::Value &doc) {
        es->validate(doc);
    }

    std::shared_ptr<EndpointSchema> getInternalSchema() {
        return es;
    };

    ~GenericSchema() {
    }
};


#endif //UBIFORM_GENERICSCHEMA_H
