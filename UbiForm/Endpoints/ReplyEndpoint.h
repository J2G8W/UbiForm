
#ifndef UBIFORM_REPLYENDPOINT_H
#define UBIFORM_REPLYENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class ReplyEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint{
public:
    ReplyEndpoint(std::shared_ptr<EndpointSchema> replySchema, std::shared_ptr<EndpointSchema> requestSchema) :
        DataReceiverEndpoint(requestSchema), DataSenderEndpoint(replySchema){

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
    }

    void listenForConnection(const char *url) override ;
    int listenForConnectionWithRV(const char *url) override;

    // SHOULD NOT DIAL FOR CONNECTION
    void dialConnection(const char *url) override;
    void closeSocket() override;
    ~ReplyEndpoint() override;
};


#endif //UBIFORM_REPLYENDPOINT_H
