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
    nng_socket * receiverSocket = nullptr;
    // Schema is shared with the parent that houses this endpoint
    std::shared_ptr<EndpointSchema>receiverSchema;
public:
    explicit DataReceiverEndpoint( std::shared_ptr<EndpointSchema>& es){
        receiverSchema = es;
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void dialConnection(const char *url) = 0;


    std::unique_ptr<SocketMessage> receiveMessage();

    void asyncReceiveMessage(void (*callb)(SocketMessage *));
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
