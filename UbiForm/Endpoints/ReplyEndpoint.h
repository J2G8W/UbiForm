
#ifndef UBIFORM_REPLYENDPOINT_H
#define UBIFORM_REPLYENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class ReplyEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
public:
    ReplyEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                  const std::string &endpointType, const std::string &endpointIdentifier = "Reply") :
            DataReceiverEndpoint(receiveSchema, endpointIdentifier, SocketType::Reply, endpointType),
            DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Reply, endpointType) {

        senderSocket = new nng_socket;
        openEndpoint();
    }

    void listenForConnection(const char *base, int port) override;

    int listenForConnectionWithRV(const char *base, int port) override;

    // SHOULD NOT DIAL FOR CONNECTION
    void dialConnection(const char *url) override;

    void closeSocket() override;
    void openEndpoint() override;

    void invalidateEndpoint() override{
        DataSenderEndpoint::endpointState = EndpointState::Invalid;
        DataReceiverEndpoint::endpointState = EndpointState::Invalid;
    }

    ~ReplyEndpoint() override;

};


#endif //UBIFORM_REPLYENDPOINT_H
