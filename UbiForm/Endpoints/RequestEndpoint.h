
#ifndef UBIFORM_REQUESTENDPOINT_H
#define UBIFORM_REQUESTENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class RequestEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint{
public:
    RequestEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                    const std::string &endpointType, const std::string &endpointIdentifier = "Request") :
            DataReceiverEndpoint(receiveSchema, endpointIdentifier, SocketType::Request, endpointType),
            DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Request, endpointType){
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
