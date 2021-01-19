#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {

public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema, const std::string &endpointType,
                               const std::string &endpointIdentifier = "Publisher") :
            DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Publisher, endpointType) {
        senderSocket = new nng_socket;
        openEndpoint();
    }


    void openEndpoint() override;

    void invalidateEndpoint() override {
        DataSenderEndpoint::endpointState = EndpointState::Invalid;
    }

    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
