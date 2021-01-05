#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "../SocketMessage.h"
#include "../Utilities/SystemEnums.h"


class EndpointSchema {
    friend class ComponentManifest;
private:
    rapidjson::SchemaDocument * schema;
    rapidjson::Value * JSON_rep;
    rapidjson::MemoryPoolAllocator<> * allocator;
    bool responsibleForJson;

    void changeSchema(){
        delete schema;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
    }


public:
    // Note that the passed in pointer is up to parent to handle memory
    explicit EndpointSchema(rapidjson::Value *doc, rapidjson::MemoryPoolAllocator<> & al) {
        allocator = &al;
        JSON_rep = doc;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
        responsibleForJson = false;
    }
    EndpointSchema(){
        rapidjson::Document * d = new rapidjson::Document();
        allocator = &(d->GetAllocator());
        d->SetObject();
        d->AddMember("type", rapidjson::Value("object",*allocator), *allocator);
        d->AddMember("properties",rapidjson::Value(rapidjson::kObjectType), *allocator);
        d->AddMember("required", rapidjson::Value(rapidjson::kArrayType), *allocator);
        JSON_rep = d;
        schema = new rapidjson::SchemaDocument (*JSON_rep);
        responsibleForJson = true;
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
    void removeProperty(const std::string& name);

    void addRequired(const std::string& name);
    void removeRequired(const std::string &name);

    void setArrayType(const std::string& name, ValueType type);
    void setArrayObject(const std::string& name, EndpointSchema& es);
    void setSubObject(const std::string& name, EndpointSchema& es);

    std::string stringify();

    ~EndpointSchema(){
        delete schema;
        if (responsibleForJson){
            // Recast to document so we can do proper cleanup AS a doc rather than as a value
            rapidjson::Document * JSON_doc = static_cast<rapidjson::Document*>(JSON_rep);
            delete JSON_doc;
        }
        // The JSON_rep pointer is handled by parent
    }


};


#endif //UBIFORM_ENDPOINTSCHEMA_H
