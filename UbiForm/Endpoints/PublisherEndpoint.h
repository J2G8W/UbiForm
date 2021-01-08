#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {

public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema, const std::string& endpointIdentifier="Publisher") :
        DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Publisher){
        senderSocket = new nng_socket ;
        int rv;
        if ((rv = nng_pub0_open(senderSocket)) != 0) {
            throw NngError(rv, "Creation of publisher socket");
        }else{
            DataSenderEndpoint::socketOpen = true;
        }
    }

    void listenForConnection(const char* url) override;
    int listenForConnectionWithRV(const char *url) override;

    void closeSocket() override;

    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
