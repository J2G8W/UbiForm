#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H


#include <nng/nng.h>
#include "../SocketMessage.h"
#include "EndpointSchema.h"

class DataSenderEndpoint {
protected:
    nng_socket * senderSocket{};
    EndpointSchema senderSchema;
public:
    explicit DataSenderEndpoint( rapidjson::Document doc) : senderSchema(doc) {};

    virtual void listenForConnection(const char *url) = 0;
    void sendMessage(SocketMessage &s);
};


#endif //UBIFORM_DATASENDERENDPOINT_H
