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