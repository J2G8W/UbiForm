#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H


#include <unistd.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {

public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema):
    DataReceiverEndpoint(receiveSchema), DataSenderEndpoint(sendSchema){}

    void listenForConnection(const char *url) override ;
    void dialConnection(const char *url) override;

    // Destructor waits a short time before closing socket such that any unsent messages are released
    ~PairEndpoint() {
        int rv;
        // Make sure that the messages are flushed
        sleep(1);
        // We only have one actual socket so only need to close 1
        if ((rv = nng_close(*receiverSocket)) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
    }
};


#endif //UBIFORM_PAIRENDPOINT_H
