#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
private:
    static void streamReceiveData(PairEndpoint *endpoint, std::ostream *stream);
    static void streamSendData(PairEndpoint *endpoint, std::istream *stream, std::streamsize blockSize,
                               bool holdWhenStreamEmpty);

    bool receiverThreadNeedsClosing = false;
    bool receiverThreadEnded = true;
    std::thread receiverStreamingThread;

    bool senderThreadNeedsClosing = false;
    bool senderThreadEnded = true;
    std::thread senderStreamingThread;
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

    void closeEndpoint() override;

    void openEndpoint() override;

    void invalidateEndpoint() override {
        endpointState = EndpointState::Invalid;
    }

    std::unique_ptr<SocketMessage> receiveStream(std::ostream &outputStream);

    bool getReceiverThreadEnded(){return receiverThreadEnded;}

    void sendStream(std::istream &input, std::streamsize blockSize, bool holdWhenStreamEmpty,
                    SocketMessage &initialMessage);
    bool getSenderThreadEnded(){return senderThreadEnded;}


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
