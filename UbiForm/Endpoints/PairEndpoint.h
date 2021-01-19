#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
private:

public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                 const std::string &endpointType, const std::string &endpointIdentifier = "Pair") :
            DataReceiverEndpoint(receiveSchema, endpointIdentifier, SocketType::Pair, endpointType),
            DataSenderEndpoint(sendSchema, endpointIdentifier, SocketType::Pair, endpointType) {

        senderSocket = new nng_socket;
        openEndpoint();
    }

    void listenForConnection(const char *base, int port) override;

    int listenForConnectionWithRV(const char *base, int port) override;

    void dialConnection(const char *url) override;

    void closeSocket() override;

    void openEndpoint() override;

    void invalidateEndpoint() override{
        DataSenderEndpoint::endpointState = EndpointState::Invalid;
        DataReceiverEndpoint::endpointState = EndpointState::Invalid;
    }


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
