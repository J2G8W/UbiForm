#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include "../SocketMessage.h"
#include "EndpointSchema.h"

class DataReceiverEndpoint {
protected:
    // Socket is initialised in extending class
    nng_socket * receiverSocket;
    std::shared_ptr<EndpointSchema>receiverSchema;
public:
    explicit DataReceiverEndpoint( std::shared_ptr<EndpointSchema>& es){
        receiverSchema = es;
    };

    virtual void dialConnection(const char *url) = 0;
    std::unique_ptr<SocketMessage> receiveMessage();
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
