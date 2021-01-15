#ifndef UBIFORM_PUBLISHERENDPOINT_H
#define UBIFORM_PUBLISHERENDPOINT_H


#include "nng/nng.h"
#include "nng/protocol/pubsub0/pub.h"

#include "DataSenderEndpoint.h"

class PublisherEndpoint : public DataSenderEndpoint {

public:
    explicit PublisherEndpoint(std::shared_ptr<EndpointSchema> sendSchema, const std::string &endpointType,
                               const std::string &endpointIdentifier = "Publisher") :
        DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Publisher, endpointType){
        senderSocket = new nng_socket ;
        int rv;
        if ((rv = nng_pub0_open(senderSocket)) != 0) {
            throw NngError(rv, "Creation of publisher socket");
        }else{
            DataSenderEndpoint::socketOpen = true;
        }
    }

    void listenForConnection(const char *base, int port) override;
    int listenForConnectionWithRV(const char *base, int port) override;

    void closeSocket() override;

    ~PublisherEndpoint();
};


#endif //UBIFORM_PUBLISHERENDPOINT_H
