#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H

#include <memory>

#include <nng/nng.h>
#include "../SocketMessage.h"

#include "EndpointSchema.h"

class DataSenderEndpoint {
protected:
    // Socket is initialised in extending class
    nng_socket * senderSocket;
    std::shared_ptr<EndpointSchema>senderSchema;
public:
    explicit DataSenderEndpoint( std::shared_ptr<EndpointSchema>& es){
        senderSchema = es;
    };

    virtual void listenForConnection(const char *url) = 0;
    void sendMessage(SocketMessage &s);
};


#endif //UBIFORM_DATASENDERENDPOINT_H
