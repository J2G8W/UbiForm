#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
private:

public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema, const std::string& endpointIdentifier="Pair"):
        DataReceiverEndpoint(receiveSchema,endpointIdentifier,SocketType::Pair),
        DataSenderEndpoint(sendSchema,endpointIdentifier, SocketType::Pair){

        senderSocket = new nng_socket;

        int rv;
        if ((rv = nng_pair0_open(senderSocket)) != 0) {
            throw NngError(rv, "Making pair connection");
        }else{
            DataReceiverEndpoint::socketOpen = true;
            DataSenderEndpoint::socketOpen = true;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
    }

    void listenForConnection(const char *base, int port) override ;
    int listenForConnectionWithRV(const char *base, int port) override;
    void dialConnection(const char *url) override;
    void closeSocket() override;


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
