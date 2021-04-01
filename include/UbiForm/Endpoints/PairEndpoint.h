#ifndef UBIFORM_PAIRENDPOINT_H
#define UBIFORM_PAIRENDPOINT_H

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include "DataReceiverEndpoint.h"
#include "DataSenderEndpoint.h"

class PairEndpoint : public DataReceiverEndpoint, public DataSenderEndpoint {
private:
    static void streamReceiveData(PairEndpoint *endpoint, std::ostream *stream, endOfStreamCallback endCallback,
                                  void *userData);
    static void
    streamSendData(PairEndpoint *endpoint, std::istream *stream, std::streamsize blockSize, bool holdWhenStreamEmpty,
                   endOfStreamCallback endCallback, void *userData);

    bool receiverThreadNeedsClosing = false;
    bool receiverThreadEnded = true;
    std::thread receiverStreamingThread;

    bool senderThreadNeedsClosing = false;
    bool senderThreadEnded = true;
    std::thread senderStreamingThread;
public:
    PairEndpoint(std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema,
                 const std::string &endpointType, const std::string &endpointIdentifier = "Pair",
                 endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
                 Endpoint(endpointIdentifier, ConnectionParadigm::Pair, endpointType,
                          startupFunction, extraData),
            DataReceiverEndpoint(receiveSchema),
            DataSenderEndpoint(sendSchema) {

        senderSocket = new nng_socket;
        openEndpoint();
    }

    void listenForConnection(const std::string &base, int port) override;

    int listenForConnectionWithRV(const std::string &base, int port) override;

    void dialConnection(const std::string &url) override;

    void closeEndpoint() override;

    void openEndpoint() override;


    std::unique_ptr<EndpointMessage>
    receiveStream(std::ostream &outputStream, endOfStreamCallback endCallback, void *userData);

    bool getReceiverThreadEnded(){return receiverThreadEnded;}

    void
    sendStream(std::istream &input, std::streamsize blockSize, bool holdWhenStreamEmpty, EndpointMessage &initialMessage,
               endOfStreamCallback endCallback, void *userData);
    bool getSenderThreadEnded(){return senderThreadEnded;}


    ~PairEndpoint();
};


#endif //UBIFORM_PAIRENDPOINT_H
