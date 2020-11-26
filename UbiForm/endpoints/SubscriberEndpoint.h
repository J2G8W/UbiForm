#ifndef UBIFORM_SUBSCRIBERENDPOINT_H
#define UBIFORM_SUBSCRIBERENDPOINT_H


#include <memory>

#include "EndpointSchema.h"
#include "DataReceiverEndpoint.h"

class SubscriberEndpoint : public DataReceiverEndpoint {
public:
    explicit SubscriberEndpoint(std::shared_ptr<EndpointSchema> receiveSchema) : DataReceiverEndpoint(receiveSchema){
        receiverSocket = new nng_socket;
    }
    void dialConnection(const char *url) override;

    ~SubscriberEndpoint();
};




#endif //UBIFORM_SUBSCRIBERENDPOINT_H
