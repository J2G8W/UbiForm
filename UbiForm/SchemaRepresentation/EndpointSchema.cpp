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
        errorText << "Invalid schema property: " << sb.GetString();
        errorText << "\nInvalid keyword: " <<  validator.GetInvalidSchemaKeyword();
        errorText << "\nSchema: " << this->stringify();
        errorText << "\nDocument: " << stringifyValue((rapidjson::Value&) doc);
        throw ValidationError(errorText.str());
    }
}

void EndpointSchema::completeUpdate(rapidjson::Value &doc) {
    JSON_rep->RemoveAllMembers();
    JSON_rep->CopyFrom(doc, *allocator);
    changeSchema();
}

SocketMessage *EndpointSchema::getSchemaObject() {
    auto * returnObject = new SocketMessage(*JSON_rep, true);
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
    else{throw ValidationError("No valid type in the schema for field: " + fieldName);}
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

void EndpointSchema::addProperty(const std::string& name, ValueType type) {
    auto properties = (*JSON_rep)["properties"].GetObject();
    rapidjson::Value newValue;
    newValue.SetString(convertValueType(type), *allocator);
    if (properties.HasMember(name)){
        properties[name].GetObject()["type"] = newValue;
    }else{
        rapidjson::Value newObject(rapidjson::kObjectType);
        newObject.AddMember(rapidjson::Value("type").Move(), newValue, *allocator);
        rapidjson::Value nameValue(name, *allocator);
        properties.AddMember(nameValue,newObject,*allocator);
    }
    changeSchema();
}

void EndpointSchema::removeProperty(const std::string& name){
    auto properties = (*JSON_rep)["properties"].GetObject();
    if (properties.HasMember(name)){
        properties.RemoveMember(name);
    }
    changeSchema();
}

void EndpointSchema::removeRequired(const std::string &name){
    bool changeMade = false;
    if (JSON_rep->HasMember("required") && (*JSON_rep)["required"].IsArray()){
        auto jsonArray = (*JSON_rep)["required"].GetArray();
        // Rapidjson issue 1316 shows method here.
        // We don't iterate in for loop as we don't want to "double jump" on erase
        for(auto itr = jsonArray.Begin(); itr != jsonArray.End() ; ){
            if (strncmp(name.c_str(),itr->GetString(), name.size()) == 0){
                itr = jsonArray.Erase(itr);
                changeMade = true;
            }else{
                itr ++;
            }
        }
    }
    if (changeMade){changeSchema();}
}

void EndpointSchema::addRequired(const std::string &name) {
    bool changeNeeded = true;
    if (JSON_rep->HasMember("required") && (*JSON_rep)["required"].IsArray()) {
        auto jsonArray = (*JSON_rep)["required"].GetArray();
        for (auto &v : jsonArray){
            if (strncmp(name.c_str(), v.GetString(), name.size()) == 0){
                changeNeeded = false;
                break;
            }
        }
        if (changeNeeded){
            rapidjson::Value v(name, *allocator);
            jsonArray.PushBack(v, *allocator);
            changeSchema();
        }
    }
}

void EndpointSchema::setArrayType(const std::string &name, ValueType type) {
    auto properties = (*JSON_rep)["properties"].GetObject();

    addProperty(name, ValueType::Array);

    rapidjson::Value items(rapidjson::kObjectType);
    items.AddMember(rapidjson::Value("type", *allocator).Move(),
                    rapidjson::Value(convertValueType(type), *allocator).Move(),
                    *allocator);
    if (properties[name].HasMember("items")){
        properties[name].GetObject()["items"] = items;

    }else {
        properties[name].AddMember("items", items, *allocator);
    }
    changeSchema();
}

void EndpointSchema::setArrayObject(const std::string &name, EndpointSchema &es) {
    auto properties = (*JSON_rep)["properties"].GetObject();
    addProperty(name, ValueType::Array);

    rapidjson::Value items(rapidjson::kObjectType);
    items.AddMember("type","object", *allocator);

    rapidjson::Value subObjectProperties(rapidjson::kObjectType);
    subObjectProperties.CopyFrom((*es.JSON_rep)["properties"],*allocator);
    items.AddMember("properties",subObjectProperties,*allocator);

    rapidjson::Value subObjectRequired(rapidjson::kArrayType);
    subObjectRequired.CopyFrom((*es.JSON_rep)["required"],*allocator);
    items.AddMember("required", subObjectRequired, *allocator);

    if (properties[name].HasMember("items")){
        properties[name].GetObject()["items"] = items;
    }else {
        properties[name].AddMember("items", items, *allocator);
    }
    changeSchema();
}


void EndpointSchema::setSubObject(const std::string &name, EndpointSchema &es) {
    auto properties = (*JSON_rep)["properties"].GetObject();

    addProperty(name, ValueType::Object);
    rapidjson::Value subObjectProperties(rapidjson::kObjectType);
    subObjectProperties.CopyFrom((*es.JSON_rep)["properties"],*allocator);

    if(properties[name].HasMember("properties")){
        properties[name].GetObject()["properties"] = subObjectProperties;
    }else {
        properties[name].AddMember("properties", subObjectProperties, *allocator);
    }

    rapidjson::Value subObjectRequired(rapidjson::kArrayType);
    subObjectRequired.CopyFrom((*es.JSON_rep)["required"],*allocator);
    if(properties[name].HasMember("required")){
        properties[name].GetObject()["required"] = subObjectRequired;
    }else {
        properties[name].AddMember("required", subObjectRequired, *allocator);
    }
    changeSchema();
}

std::string EndpointSchema::stringify() {
    return stringifyValue(*JSON_rep);
}
