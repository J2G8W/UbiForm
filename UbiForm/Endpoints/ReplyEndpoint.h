
#ifndef UBIFORM_REPLYENDPOINT_H
#define UBIFORM_REPLYENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class ReplyEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint{
public:
    ReplyEndpoint(std::shared_ptr<EndpointSchema> replySchema, std::shared_ptr<EndpointSchema> requestSchema, const std::string& endpointIdentifier="Reply") :
        DataReceiverEndpoint(requestSchema, endpointIdentifier, SocketType::Reply),
        DataSenderEndpoint(replySchema, endpointIdentifier, SocketType::Reply){

        senderSocket = new nng_socket;

        int rv;
        if ((rv = nng_rep0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open RD connection request socket");
        }else{
            DataReceiverEndpoint::socketOpen = true;
            DataSenderEndpoint::socketOpen = true;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
        // By default we have an infinite timeout
        setTimeout(-1);
    }

    void listenForConnection(const char *base, int port) override ;
    int listenForConnectionWithRV(const char *base, int port) override;

    // SHOULD NOT DIAL FOR CONNECTION
    void dialConnection(const char *url) override;
    void closeSocket() override;
    ~ReplyEndpoint() override;

    void setTimeout(int timeout);
};


#endif //UBIFORM_REPLYENDPOINT_H
