#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {
private:
    bool socketOpen = false;

public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema) : DataSenderEndpoint(sendSchema){
        senderSocket = new nng_socket ;
        int rv;
        if ((rv = nng_pub0_open(senderSocket)) != 0) {
            throw NngError(rv, "Creation of publisher socket");
        }else{
            socketOpen = true;
        }
    }

    void listenForConnection(const char* url) override;
    int listenForConnectionWithRV(const char *url) override;

    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
