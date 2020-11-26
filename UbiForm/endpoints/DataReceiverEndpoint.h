#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include "../SocketMessage.h"
#include "EndpointSchema.h"

class DataReceiverEndpoint {
protected:
    nng_socket * receiverSocket = new nng_socket;
    std::shared_ptr<EndpointSchema>receiverSchema;
public:
    explicit DataReceiverEndpoint( std::shared_ptr<EndpointSchema>& es){
        receiverSchema = es;
    };

    virtual void dialConnection(const char *url) = 0;
    std::unique_ptr<SocketMessage> receiveMessage();
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
