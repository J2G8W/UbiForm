#ifndef UBIFORM_SYSTEMENUMS_H
#define UBIFORM_SYSTEMENUMS_H


#include <string>

// NOTE THAT these strings are defined in the various schemas in SystemSchemas

#define PAIR "pair"
#define PUBLISHER "publisher"
#define SUBSCRIBER "subscriber"
#define REPLY "reply"
#define REQUEST "request"

enum SocketType{
    Pair,Publisher,Subscriber, Reply, Request
};
std::string convertSocketType(SocketType st);

enum ValueType{
    Number, String, Boolean, Object, Array, Null
};
std::string convertValueType(ValueType vt);

enum SystemSchemaName{
    componentManifest,endpointCreationRequest, endpointCreationResponse,
    additionRequest,additionResponse,byIdRequest,byIdResponse,
    bySchemaRequest, bySchemaResponse, componentIdsRequest, componentIdsResponse,updateRequest
};


#endif //UBIFORM_SYSTEMENUMS_H
