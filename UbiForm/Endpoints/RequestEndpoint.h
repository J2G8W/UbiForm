
#ifndef UBIFORM_REQUESTENDPOINT_H
#define UBIFORM_REQUESTENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class RequestEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint{
public:
    RequestEndpoint(std::shared_ptr<EndpointSchema> replySchema, std::shared_ptr<EndpointSchema> requestSchema, const std::string& endpointIdentifier="Request"):
            DataReceiverEndpoint(replySchema, endpointIdentifier, SocketType::Request),
            DataSenderEndpoint(requestSchema, endpointIdentifier, SocketType::Request){
        senderSocket = new nng_socket;
    }

    // SHOULD NOT LISTEN FOR CONNECTION
    void listenForConnection(const char *base, int port) override ;
    int listenForConnectionWithRV(const char *base, int port) override;

    // Milliseconds
    void setTimeout(int timeout);

    void dialConnection(const char *url) override;
    void closeSocket() override;
    ~RequestEndpoint() override;
};


#endif //UBIFORM_REQUESTENDPOINT_H
