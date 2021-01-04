#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "../SocketMessage.h"

class EndpointSchema {
private:
    rapidjson::SchemaDocument * schema;
    rapidjson::Value * JSON_rep;
    rapidjson::MemoryPoolAllocator<> & allocator;

public:
    explicit EndpointSchema(rapidjson::Value *doc, rapidjson::MemoryPoolAllocator<> & al) : allocator(al) {
        JSON_rep = doc;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
    }

    void updateSchema(rapidjson::Value &doc);

    SocketMessage * getSchemaObject();

    void validate(const SocketMessage &messageToValidate);
    void validate(const rapidjson::Value &doc);
};


#endif //UBIFORM_ENDPOINTSCHEMA_H
