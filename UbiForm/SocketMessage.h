#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include <iostream>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "general_functions.h"


// Note that we need RAPIDJSON_HAS_STDSTRING turned on
class SocketMessage {
private:
    rapidjson::Document JSON_document;

public:
    SocketMessage(): JSON_document(){
        JSON_document.SetObject();
    };

    void addMember(const std::string &attributeName, const std::string &value){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value valueContainer(value, JSON_document.GetAllocator());
        JSON_document.AddMember(key, valueContainer, JSON_document.GetAllocator());
    }

    void addMember(const std::string &attributeName, int value){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        JSON_document.AddMember(key, value, JSON_document.GetAllocator());
    }
    void addMember(const std::string &attributeName, bool value){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        JSON_document.AddMember(key, value, JSON_document.GetAllocator());
    }




    char * stringify(){return stringifyDocument(JSON_document);};

};


#endif //UBIFORM_SOCKETMESSAGE_H
