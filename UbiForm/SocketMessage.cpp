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
