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

ValueType EndpointSchema::getValueType(const std::string& fieldName) {
    auto properties = (*JSON_rep)["properties"].GetObject();

    if(!(properties.HasMember(fieldName))){ throw AccessError("There is no field of name - " + fieldName);}
    if(!properties[fieldName].IsObject()){throw AccessError("That field is not an object - " + fieldName);}
    auto fieldObj = properties[fieldName].GetObject();
    if(!(fieldObj.HasMember("type"))){throw AccessError("There is no type field - " + fieldName);}
    std::string valueType = fieldObj["type"].GetString();


    if (valueType == "number"){return ValueType::Number;}
    else if (valueType == "string"){return ValueType::String;}
    else if(valueType == "boolean"){return ValueType::Boolean;}
    else if (valueType == "object"){return ValueType::Object;}
    else if (valueType == "array"){return ValueType::Array;}
    else if (valueType == "null"){return ValueType::Null;}
    else{throw ValidationError("No valid type in the schema");}
}

std::vector<std::string> EndpointSchema::getAllProperties() {
    std::vector<std::string> propertyNames;
    auto propertiesJSON = (*JSON_rep)["properties"].GetObject();
    propertyNames.reserve(propertiesJSON.MemberCount());

    for (auto& v : propertiesJSON) {
        propertyNames.emplace_back(v.name.GetString());
    }
    return propertyNames;
}

std::vector<std::string> EndpointSchema::getRequired() {
    if (!(JSON_rep->HasMember("required") && (*JSON_rep)["required"].IsArray())){
        return std::vector<std::string>();
    }
    std::vector<std::string> requiredAttributes;
    auto requiredJSON = (*JSON_rep)["required"].GetArray();
    requiredAttributes.reserve(requiredJSON.Size());

    for (auto& v : requiredJSON) {
        requiredAttributes.emplace_back(v.GetString());
    }
    return requiredAttributes;
}
