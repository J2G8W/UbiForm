#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "SocketMessage.h"

enum ValueType{
    Number, String, Boolean, Object, Array, Null
};

class EndpointSchema {
private:
    rapidjson::SchemaDocument * schema;
    rapidjson::Value * JSON_rep;
    rapidjson::MemoryPoolAllocator<> & allocator;

    void changeSchema(){
        delete schema;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
    }

public:
    // Note that the passed in pointer is up to parent to handle memory
    explicit EndpointSchema(rapidjson::Value *doc, rapidjson::MemoryPoolAllocator<> & al) : allocator(al) {
        JSON_rep = doc;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
    }

    // Copy constructor
    void completeUpdate(rapidjson::Value &doc);

    SocketMessage * getSchemaObject();

    void validate(const SocketMessage &messageToValidate);
    void validate(const rapidjson::Value &doc);

    ValueType getValueType(const std::string& fieldName);

    std::vector<std::string> getRequired();
    std::vector<std::string> getAllProperties();

    void addProperty(const std::string& name, ValueType type);

    ~EndpointSchema(){
        delete schema;
        // The JSON_rep pointer is handled by parent
    }
};


#endif //UBIFORM_ENDPOINTSCHEMA_H
