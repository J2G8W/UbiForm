#include "SocketMessage.h"

SocketMessage::~SocketMessage() = default;

int SocketMessage::getInteger(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsInt()){
        return JSON_document[attributeName].GetInt();
    }else{
        throw std::logic_error("The message does not have an element of that type");
    }
}
bool SocketMessage::getBoolean(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsBool()){
        return JSON_document[attributeName].GetBool();
    }else{
        throw std::logic_error("The message does not have an element of that type");
    }
}
std::string SocketMessage::getString(const std::string &attributeName){
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsString()){
        return std::string(JSON_document[attributeName].GetString());
    }else{
        throw std::logic_error("The message does not have an element of that type");
    }
}

SocketMessage* SocketMessage::getObject(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsObject()){
        return new SocketMessage(JSON_document[attributeName]);
    }else {
        throw std::logic_error("The message does not have an element of that type");
    }
}

template<>
std::vector<int> SocketMessage::getArray<int>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsArray()) {
        auto memberArray = JSON_document[attributeName].GetArray();
        std::vector<int> returnVector;
        returnVector.reserve(memberArray.Size());
        for (auto &v: memberArray) {
            assert(v.IsInt());
            returnVector.push_back(v.GetInt());
        }
        return returnVector;
    }else{
        throw std::logic_error("This message does not have element of that type");
    }
}

template<>
std::vector<bool> SocketMessage::getArray<bool>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsArray()) {
        auto memberArray = JSON_document[attributeName].GetArray();
        std::vector<bool> returnVector;
        returnVector.reserve(memberArray.Size());
        for (auto &v: memberArray) {
            assert(v.IsBool());
            returnVector.push_back(v.GetBool());
        }
        return returnVector;
    }else{
        throw std::logic_error("This message does not have element of that type");
    }
}

template<>
std::vector<std::string> SocketMessage::getArray<std::string>(const std::string &attributeName) {
    if (JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsArray()) {
        auto memberArray = JSON_document[attributeName].GetArray();
        std::vector<std::string> returnVector;
        returnVector.reserve(memberArray.Size());
        for (auto &v: memberArray) {
            assert(v.IsString());
            returnVector.emplace_back(v.GetString());
        }
        return returnVector;
    }else{
        throw std::logic_error("This message does not have element of that type");
    }
}

