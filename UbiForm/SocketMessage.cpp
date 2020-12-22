#include "SocketMessage.h"

SocketMessage::~SocketMessage() = default;

int SocketMessage::getInteger(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName)){
        if(JSON_document[attributeName].IsInt()){
            return JSON_document[attributeName].GetInt();
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type integer");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}
bool SocketMessage::getBoolean(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName)){
        if(JSON_document[attributeName].IsBool()) {
            return JSON_document[attributeName].GetBool();
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type boolean");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}
std::string SocketMessage::getString(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName)){
        if(JSON_document[attributeName].IsString()) {
            return JSON_document[attributeName].GetString();
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type string");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}

SocketMessage* SocketMessage::getObject(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName)){
        if(JSON_document[attributeName].IsObject()) {
            return new SocketMessage(JSON_document[attributeName]);
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type boolean");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}


SocketMessage::SocketMessage(const char *jsonString) {
    rapidjson::StringStream stream(jsonString);
    JSON_document.ParseStream(stream);
    if (JSON_document.HasParseError()){
        std::ostringstream error;
        error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
        error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
        throw ParsingError(error.str());
    }
}

void SocketMessage::addMember(const std::string &attributeName, const std::string &value) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());
    rapidjson::Value valueContainer(value, JSON_document.GetAllocator());
    addOrSwap(key, valueContainer);
}

void SocketMessage::addMember(const std::string &attributeName, const char *value) {
    addMember(attributeName, std::string(value));
}

void SocketMessage::addMember(const std::string &attributeName, int value) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());
    rapidjson::Value valueContainer(value);
    addOrSwap(key, valueContainer);
}

void SocketMessage::addMember(const std::string &attributeName, bool value) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());
    rapidjson::Value valueContainer(value);
    addOrSwap(key, valueContainer);
}

void SocketMessage::addMember(const std::string &attributeName, SocketMessage &socketMessage) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());
    rapidjson::Value v;
    v.CopyFrom(socketMessage.JSON_document,JSON_document.GetAllocator());
    addOrSwap(key,v);
}

void SocketMessage::setNull(const std::string &attributeName) {
    rapidjson::Value v;
    v.SetNull();
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());
    addOrSwap(key, v);
}

void SocketMessage::addMember(const std::string &attributeName, const std::vector<SocketMessage *>& inputArray) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());

    rapidjson::Value valueArray(rapidjson::kArrayType);
    valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
    for (auto object : inputArray){
        rapidjson::Value v;
        v.SetObject();
        v.CopyFrom(object->JSON_document, JSON_document.GetAllocator());
        valueArray.PushBack(v, JSON_document.GetAllocator());
    }
    addOrSwap(key,valueArray);
}

void SocketMessage::addMember(const std::string &attributeName, const std::vector<std::string>& inputArray) {
    rapidjson::Value key(attributeName, JSON_document.GetAllocator());

    rapidjson::Value valueArray(rapidjson::kArrayType);
    valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
    for (const std::string& item : inputArray){
        rapidjson::Value v(item,JSON_document.GetAllocator());
        valueArray.PushBack(v, JSON_document.GetAllocator());
    }
    addOrSwap(key,valueArray);
}


template<>
std::vector<int> SocketMessage::getArray<int>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName)) {
        if (JSON_document[attributeName].IsArray()) {
            auto memberArray = JSON_document[attributeName].GetArray();
            std::vector<int> returnVector;
            returnVector.reserve(memberArray.Size());
            for (auto &v: memberArray) {
                if (!v.IsInt()){throw AccessError("Array contains a non-integer value");}
                returnVector.push_back(v.GetInt());
            }
            return returnVector;
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type array");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}

template<>
std::vector<bool> SocketMessage::getArray<bool>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName)) {
        if (JSON_document[attributeName].IsArray()) {
            auto memberArray = JSON_document[attributeName].GetArray();
            std::vector<bool> returnVector;
            returnVector.reserve(memberArray.Size());
            for (auto &v: memberArray) {
                if (!v.IsBool()){throw AccessError("Array contains a non-boolean value");}
                returnVector.push_back(v.GetBool());
            }
            return returnVector;
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type array");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}

template<>
std::vector<std::string> SocketMessage::getArray<std::string>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName)) {
        if (JSON_document[attributeName].IsArray()) {
            auto memberArray = JSON_document[attributeName].GetArray();
            std::vector<std::string> returnVector;
            returnVector.reserve(memberArray.Size());
            for (auto &v: memberArray) {
                if (!v.IsString()){throw AccessError("Array contains a non-string value");}
                returnVector.emplace_back(v.GetString());
            }
            return returnVector;
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type array");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}

template<>
std::vector<SocketMessage *> SocketMessage::getArray<SocketMessage *>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName)) {
        if (JSON_document[attributeName].IsArray()) {
            auto memberArray = JSON_document[attributeName].GetArray();
            std::vector<SocketMessage *> returnVector;
            returnVector.reserve(memberArray.Size());
            for (auto &v: memberArray) {
                if (!v.IsObject()){throw AccessError("Array contains a non-object value");}
                returnVector.push_back(new SocketMessage(v));
            }
            return returnVector;
        }else{
            throw AccessError("Attribute " + attributeName + "exists but not type array");
        }
    }else{
        throw AccessError("The message has no attribute " + attributeName);
    }
}