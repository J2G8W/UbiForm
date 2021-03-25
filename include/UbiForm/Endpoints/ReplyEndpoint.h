
#ifndef UBIFORM_REPLYENDPOINT_H
#define UBIFORM_REPLYENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>

#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class ReplyEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
public:
    ReplyEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                  const std::string &endpointType, const std::string &endpointIdentifier = "Reply",
                  endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
            Endpoint( endpointIdentifier, SocketType::Reply, endpointType,
                      startupFunction,extraData),
            DataReceiverEndpoint(receiveSchema),
            DataSenderEndpoint(sendSchema) {

        senderSocket = new nng_socket;
        openEndpoint();
    }

    void listenForConnection(const std::string &base, int port) override;

    int listenForConnectionWithRV(const std::string &base, int port) override;

    // SHOULD NOT DIAL FOR CONNECTION
    void dialConnection(const std::string &url) override;

    void closeEndpoint() override;

    void openEndpoint() override;


    ~ReplyEndpoint() override;

};


#endif //UBIFORM_REPLYENDPOINT_H
