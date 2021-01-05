#ifndef UBIFORM_SUBSCRIBERENDPOINT_H
#define UBIFORM_SUBSCRIBERENDPOINT_H


#include <memory>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include "../SchemaRepresentation/EndpointSchema.h"
#include "DataReceiverEndpoint.h"

class SubscriberEndpoint : public DataReceiverEndpoint {

public:
    explicit SubscriberEndpoint(std::shared_ptr<EndpointSchema> receiveSchema) : DataReceiverEndpoint(receiveSchema){
        receiverSocket = new nng_socket;
        int rv;
        if ((rv = nng_sub0_open(receiverSocket)) != 0) {
            throw NngError(rv, "Opening subscriber socket");
        }else{
            socketOpen = true;
        }
    }
    void dialConnection(const char *url) override;
    void closeSocket() override;
    ~SubscriberEndpoint();
};




#endif //UBIFORM_SUBSCRIBERENDPOINT_H
