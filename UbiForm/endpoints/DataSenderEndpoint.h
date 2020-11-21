#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H


#include <nng/nng.h>
#include "../SocketMessage.h"
#include "EndpointSchema.h"

class DataSenderEndpoint {
protected:
    nng_socket * senderSocket = new nng_socket ;
    EndpointSchema * senderSchema;
public:
    explicit DataSenderEndpoint( EndpointSchema *es) : senderSchema(es) {};

    virtual void listenForConnection(const char *url) = 0;
    void sendMessage(SocketMessage &s);
};


#endif //UBIFORM_DATASENDERENDPOINT_H
