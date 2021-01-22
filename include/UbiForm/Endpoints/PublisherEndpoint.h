#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {

public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema, const std::string &endpointType,
                               const std::string &endpointIdentifier = "Publisher",
                               endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
                               Endpoint(endpointIdentifier, SocketType::Publisher, endpointType,
                                        startupFunction, extraData)
                               , DataSenderEndpoint(sendSchema) {
        senderSocket = new nng_socket;
        openEndpoint();
    }


    void openEndpoint() override;


    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
