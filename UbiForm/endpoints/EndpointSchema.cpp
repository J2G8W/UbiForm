#include "EndpointSchema.h"


// Validate a socket message against the manifest
void EndpointSchema::validate(const SocketMessage &messageToValidate) {

    validate(messageToValidate.JSON_document);
}

void EndpointSchema::validate(const rapidjson::Value &doc) {
    rapidjson::SchemaValidator validator(*schema);
    if (!doc.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Raise exception
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);

        std::ostringstream errorText;
        errorText << "Invalid schema property: " << sb.GetString() << std::endl ;
        errorText << "Invalid keyword: " <<  validator.GetInvalidSchemaKeyword() ;
        throw ValidationError(errorText.str());
    }
}

void EndpointSchema::updateSchema(rapidjson::Value &doc) {
    delete schema;
    JSON_rep->RemoveAllMembers();
    JSON_rep->CopyFrom(doc, allocator);
    schema = new rapidjson::SchemaDocument(*JSON_rep);
}

SocketMessage *EndpointSchema::getSchemaObject() {
    auto * returnObject = new SocketMessage(*JSON_rep);
    return returnObject;
}

