
#ifndef UBIFORM_REQUESTENDPOINT_H
#define UBIFORM_REQUESTENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class RequestEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint{
public:
    RequestEndpoint(std::shared_ptr<EndpointSchema> replySchema, std::shared_ptr<EndpointSchema> requestSchema):
            DataReceiverEndpoint(replySchema), DataSenderEndpoint(requestSchema){

        senderSocket = new nng_socket;

        int rv;
        if ((rv = nng_req0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open RD connection request socket");
        }else{
            DataReceiverEndpoint::socketOpen = true;
            DataSenderEndpoint::socketOpen = true;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
    }

    // SHOULD NOT LISTEN FOR CONNECTION
    void listenForConnection(const char *url) override ;
    int listenForConnectionWithRV(const char *url) override;


    void dialConnection(const char *url) override;
    void closeSocket() override;
    ~RequestEndpoint() override;
};


#endif //UBIFORM_REQUESTENDPOINT_H
