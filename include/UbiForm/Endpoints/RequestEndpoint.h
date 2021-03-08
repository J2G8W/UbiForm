
#ifndef UBIFORM_REQUESTENDPOINT_H
#define UBIFORM_REQUESTENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class RequestEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
    nng_dialer dialer;
public:
    RequestEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                    const std::string &endpointType, const std::string &endpointIdentifier = "Request",
                    endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
                    Endpoint(endpointIdentifier, SocketType::Request, endpointType, startupFunction,extraData),
            DataReceiverEndpoint(receiveSchema),
            DataSenderEndpoint(sendSchema) {
        senderSocket = new nng_socket;
        openEndpoint();
    }

    // SHOULD NOT LISTEN FOR CONNECTION
    void listenForConnection(const char *base, int port) override;

    int listenForConnectionWithRV(const char *base, int port) override;


    void dialConnection(const char *url) override;

    void closeEndpoint() override;

    void openEndpoint() override;

    int dialOption(){
        return nng_dialer_id(dialer);
    }

    ~RequestEndpoint() override;
};


#endif //UBIFORM_REQUESTENDPOINT_H
