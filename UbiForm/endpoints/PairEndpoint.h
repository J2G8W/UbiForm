#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H


#include <unistd.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {

public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema):
    DataReceiverEndpoint(receiveSchema), DataSenderEndpoint(sendSchema){
        senderSocket = new nng_socket;
    }

    void listenForConnection(const char *url) override ;
    void dialConnection(const char *url) override;


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
