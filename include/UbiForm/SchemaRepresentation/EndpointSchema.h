#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>

#include "../EndpointMessage.h"
#include "../Utilities/SystemEnums.h"

/**
 * This class represents a JSON-schema for some endpoint in our component. It allows us to strongly type our incoming
 * and outgoing messages from an endpoint
 */
class EndpointSchema {
    friend class ComponentManifest;

private:
    rapidjson::SchemaDocument *schema;
    rapidjson::Value *JSON_rep;
    rapidjson::MemoryPoolAllocator<> *allocator;
    bool responsibleForJson;

    void changeSchema() {
        delete schema;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
    }


public:
    /**
     * Create EndpointSchema which shares memory with the given MemoryPoolAllocator. MUST NOT DELETE created object before
     * this object or we get memory access problems
     * @param doc - The rapidjson value which represents our schema
     * @param al - The memory allocator which stores the memory of the given doc
     */
    explicit EndpointSchema(rapidjson::Value *doc, rapidjson::MemoryPoolAllocator<> &al) {
        allocator = &al;
        JSON_rep = doc;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
        responsibleForJson = false;
    }

    /**
     * Create EndpointSchema which shares memory with the EndpointMessage that was input. Don't delete SocketMEssage before
     * copying this schema
     * @param sm - Parent message (don't delete before deletion of this object)
     */
    explicit EndpointSchema(EndpointMessage &sm) {
        JSON_rep = &sm.JSON_document;
        allocator = &(sm.JSON_document.GetAllocator());
        schema = new rapidjson::SchemaDocument(*JSON_rep);
        responsibleForJson = false;
    }

    /**
     * Create EndpointSchema which is responsible for its own memory
     */
    EndpointSchema() {
        rapidjson::Document *d = new rapidjson::Document();
        allocator = &(d->GetAllocator());
        d->SetObject();
        d->AddMember("type", rapidjson::Value("object", *allocator), *allocator);
        d->AddMember("properties", rapidjson::Value(rapidjson::kObjectType), *allocator);
        d->AddMember("required", rapidjson::Value(rapidjson::kArrayType), *allocator);
        JSON_rep = d;
        schema = new rapidjson::SchemaDocument(*JSON_rep);
        responsibleForJson = true;
    }

    /**
     * Completely updates our schema with some other value. NOTE that this still uses our original allocator of the EndpointSchema,
     * so the same advice about deleting still holds
     * @param doc
     */
    void completeUpdate(rapidjson::Value &doc);

    /**
     *
     * @return A copy of the schema represented as a EndpointMessage
     */
    std::unique_ptr<EndpointMessage> getSchemaObject();

    ///@{
    /**
     * We validate a given input against our schema
     * @param messageToValidate
     * @throws ValidationError when our message does not validate
     */
    void validate(const EndpointMessage &messageToValidate);

    /**
     * We validate a given input against our schema
     * @param doc
     * @throws ValidationError when our message does not validate
     */
    void validate(const rapidjson::Value &doc);
    ///@}

    /**
     * Get the type of value that the field is
     * @param fieldName
     * @return The type of value that the given field is
     * @throws AccessError when the fieldName isn't in the schema
     */
    ValueType getValueType(const std::string &fieldName);

    /**
     * Gets the required field name for this schema
     * @return Vector of required field names in schema
     */
    std::vector<std::string> getRequired();

    /**
     * Gets all the property names that the schema checks against
     * @return Vector of property name in the schema
     */
    std::vector<std::string> getAllProperties();

    /**
     * We add a property of given type to our schema
     * @param name - property name
     * @param type - the type
     */
    void addProperty(const std::string &name, ValueType type);

    /**
     * Remove property from our properties. If name is not in schema does nothing
     * @param name
     */
    void removeProperty(const std::string &name);

    /**
     * Add a property name as 'required' to our schema
     * @param name
     */
    void addRequired(const std::string &name);

    /**
     * Remove a property from the required list
     * @param name
     */
    void removeRequired(const std::string &name);

    /**
     * Set the given name to be an ARRAY OF TYPES
     * @param name
     * @param type
     */
    void setArrayType(const std::string &name, ValueType type);

    /**
     * Set the given name to be an ARRAY OF given schemas
     * @param name
     * @param es
     */
    void setArrayObject(const std::string &name, EndpointSchema &es);

    /**
     * Set the given name to be a sub object within the schema
     * @param name
     * @param es
     */
    void setSubObject(const std::string &name, EndpointSchema &es);

    /**
     * @return String representation of the schema
     */
    std::string stringify();

    /**
     * This destructor has some complexity as our EndpointSchemas sometimes own their own data.
     * Hence if the responsible for JSON flag is set, we cast our Value back to a Document and delete it, and if not
     * we just leave it as is
     */
    ~EndpointSchema() {
        delete schema;
        if (responsibleForJson) {
            // Recast to document so we can do proper cleanup AS a doc rather than as a value
            rapidjson::Document *JSON_doc = static_cast<rapidjson::Document *>(JSON_rep);
            delete JSON_doc;
        }
        // The JSON_rep pointer is handled by parent
    }


};


#endif //UBIFORM_ENDPOINTSCHEMA_H
