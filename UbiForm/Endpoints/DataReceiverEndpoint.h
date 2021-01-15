#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include "../SocketMessage.h"
#include "../SchemaRepresentation/EndpointSchema.h"

class DataReceiverEndpoint {
private:
    static void asyncCallback(void *data);

    // Never freed...
    nng_aio *uniqueEndpointAioPointer;

    // This is the data structure which we will pass into our callback
    // Note this is not part of any class, it is hidden
    struct AsyncData {
        // The uniqueEndpointAioPointer has to be initialised by C style semantics


        void (*callback)(SocketMessage *, void *);

        std::shared_ptr<EndpointSchema> endpointSchema;
        DataReceiverEndpoint* owningEndpoint;
        void *furtherUserData;

        AsyncData(void (*cb)(SocketMessage *, void *), std::shared_ptr<EndpointSchema> endpointSchema,
                  void *furtherUserData, DataReceiverEndpoint* dataReceiverEndpoint) :
                callback(cb), endpointSchema(endpointSchema) {

            // So we allocate the async_io with a pointer to our asyncCallback and a pointer to this object
            this->owningEndpoint = dataReceiverEndpoint;
            this->furtherUserData = furtherUserData;
        };
    };

protected:
    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;

    // Socket is initialised in extending class
    nng_socket *receiverSocket = nullptr;
    // Schema is shared with the parent that houses this endpoint
    std::shared_ptr<EndpointSchema> receiverSchema;

    bool socketOpen = false;
    std::string dialUrl;
public:
    explicit DataReceiverEndpoint(std::shared_ptr<EndpointSchema> &es, const std::string &endpointIdentifier,
                                  SocketType socketType, const std::string &endpointType) :
            endpointIdentifier(endpointIdentifier), socketType(socketType), endpointType(endpointType) {
        receiverSchema = es;
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void dialConnection(const char *url) = 0;

    // Standard blocking receive of message.
    std::unique_ptr<SocketMessage> receiveMessage();

    // Here we receive a message asynchronously, it accepts a function which does some work on a SocketMessage
    // NOTE that the SocketMessage should not be freed by the given func (library handles the memory management)
    // Concept is that API users will provide "furtherData" which will be given to our callback as the void* pointer
    void asyncReceiveMessage(void (*callb)(SocketMessage *, void *), void *);

    std::string getDialUrl() { return dialUrl; }

    std::string &getReceiverEndpointID() { return endpointIdentifier; }

    std::string &getReceiverEndpointType() { return endpointType; }

    virtual void closeSocket() = 0;

    virtual ~DataReceiverEndpoint() = default;

    // -1 is infinite time
    // 0+ is in milliseconds
    void setReceiveTimeout(int ms_time);
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
