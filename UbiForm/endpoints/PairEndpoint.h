#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <unistd.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
private:
    bool socketOpen = false;


public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema):
    DataReceiverEndpoint(receiveSchema), DataSenderEndpoint(sendSchema){
        senderSocket = new nng_socket;

        int rv;
        if ((rv = nng_pair0_open(senderSocket)) != 0) {
            throw NngError(rv, "Making pair connection");
        }else{
            socketOpen = true;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
    }

    void listenForConnection(const char *url) override ;
    int listenForConnectionWithRV(const char *url) override;
    void dialConnection(const char *url) override;


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
