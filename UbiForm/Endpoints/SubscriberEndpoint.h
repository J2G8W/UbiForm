#ifndef UBIFORM_SUBSCRIBERENDPOINT_H
#define UBIFORM_SUBSCRIBERENDPOINT_H


#include <memory>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>

#include "../SchemaRepresentation/EndpointSchema.h"
#include "DataReceiverEndpoint.h"

class SubscriberEndpoint : public DataReceiverEndpoint {

public:
    explicit SubscriberEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, const std::string &endpointType,
                                const std::string &endpointIdentifier = "Subscriber") :
            DataReceiverEndpoint(receiveSchema, endpointIdentifier, SocketType::Subscriber, endpointType) {
        receiverSocket = new nng_socket;
        openEndpoint();
    }

    void dialConnection(const char *url) override;

    void closeSocket() override;
    void openEndpoint() override;

    ~SubscriberEndpoint();
};


#endif //UBIFORM_SUBSCRIBERENDPOINT_H
