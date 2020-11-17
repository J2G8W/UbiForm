#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "general_functions.h"

class SocketMessage {
private:
    rapidjson::Document JSON_document;

public:
    SocketMessage(): JSON_document(){
        JSON_document.SetObject();
    };

    void addInteger(std::string &attributeName, int value){
        rapidjson::Value key(attributeName.c_str(), JSON_document.GetAllocator());
        JSON_document.AddMember(key, value, JSON_document.GetAllocator());
    }

    char * stringify(){return stringifyDocument(JSON_document);};

};


#endif //UBIFORM_SOCKETMESSAGE_H
