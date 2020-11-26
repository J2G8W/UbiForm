#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {
public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema) : DataSenderEndpoint(sendSchema){
        senderSocket = new nng_socket ;
    }

    void listenForConnection(const char* url) override;

    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
