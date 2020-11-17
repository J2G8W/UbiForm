#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "general_functions.h"

class SocketMessage {
private:
    rapidjson::Document JSON_document;

public:
    SocketMessage(): JSON_document(){};


    char * stringify(){return stringifyDocument(JSON_document);};

};


#endif //UBIFORM_SOCKETMESSAGE_H
