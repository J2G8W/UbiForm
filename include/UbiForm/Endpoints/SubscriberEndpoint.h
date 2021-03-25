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
                                const std::string &endpointIdentifier = "Subscriber",
                                endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
                                Endpoint( endpointIdentifier, SocketType::Subscriber, endpointType,
                                          startupFunction, extraData),
                                DataReceiverEndpoint(receiveSchema) {
        receiverSocket = new nng_socket;
        openEndpoint();
    }

    void dialConnection(const std::string &url) override;

    void openEndpoint() override;



    ~SubscriberEndpoint();
};


#endif //UBIFORM_SUBSCRIBERENDPOINT_H
