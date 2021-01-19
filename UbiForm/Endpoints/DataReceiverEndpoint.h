#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include "../SocketMessage.h"
#include "../SchemaRepresentation/EndpointSchema.h"

/**
 * This class represents a data receiver in our design. I have made the choice that it is only able to dial, not listen
 * as this fits with the IOT scope best. The receive of messages is the same no matter the type of endpoint BUT our dialing
 * process is different, hence why this is extended in the child classes
 */
class DataReceiverEndpoint {
private:
    static void asyncCallback(void *data);

    // Never freed...
    nng_aio *uniqueEndpointAioPointer = nullptr;

    /**
     * This is the data structure used only internally for our async functions, it can have arbitrary data in it with
     * the void*.
     */
    struct AsyncData {
        void (*callback)(SocketMessage *, void *);

        std::shared_ptr<EndpointSchema> endpointSchema;
        DataReceiverEndpoint* owningEndpoint;
        void *furtherUserData;

        AsyncData(void (*cb)(SocketMessage *, void *), std::shared_ptr<EndpointSchema> endpointSchema,
                  void *furtherUserData, DataReceiverEndpoint* dataReceiverEndpoint) :
                callback(cb), endpointSchema(endpointSchema) {

            this->owningEndpoint = dataReceiverEndpoint;
            this->furtherUserData = furtherUserData;
        };
    };

protected:
    // These are all used by child classes in reporting errors
    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;

    // Socket is initialised in extending class
    nng_socket *receiverSocket = nullptr;
    // Schema is shared with the parent that houses this endpoint
    std::shared_ptr<EndpointSchema> receiverSchema;

    EndpointState endpointState = EndpointState::Closed;
    std::string dialUrl = "";
public:
    explicit DataReceiverEndpoint(std::shared_ptr<EndpointSchema> &es, const std::string &endpointIdentifier,
                                  SocketType socketType, const std::string &endpointType) :
            endpointIdentifier(endpointIdentifier), socketType(socketType), endpointType(endpointType) {
        receiverSchema = es;
    };

    /**
    * Extended by child classes, to dial external connections
    * @param url
     * @throws NngError when we are unable to dial the url properly
    */
    virtual void dialConnection(const char *url);

    /**
     * Blocking receive of a message
     * @return The message that was sent
     * @throws NngError when the underlying connection, fails or timeouts or some other error
     * @throws ValidationError when the message we receive does not conform to our schema
     * @throws SocketOpenError when our socket has been closed
     */
    std::unique_ptr<SocketMessage> receiveMessage();


    /**
     * We receive a message asynchronously, accepting a function which does work a SocketMessage. The function handles
     * the memory management of the SocketMessage. Additionally we are able to pass in arbitrary data as additionalData
     * which is then accessible as the second attribute in the called function
     * @param callb - The function which is called when a SocketMessage is received - DON'T BE A BLOCKING FUNCTION or we
     * can get deadlock scenarios
     * @param additionalData - Extra data which you want to be available in the call back
     * @throws SocketOpenError - When the socket is not open
     */
    void asyncReceiveMessage(void (*callb)(SocketMessage *, void *), void * additionalData);

    /**
     * Get the URL we are currently dialled onto
     * @return dialUrl
     */
    std::string getDialUrl() { return dialUrl; }

    std::string &getReceiverEndpointID() { return endpointIdentifier; }

    std::string &getReceiverEndpointType() { return endpointType; }

    /**
     * This function close the socket, it is implemented in each individual endpoint as we do different things for different
     * ones
     */
    virtual void closeSocket() = 0;

    virtual void openEndpoint() = 0;

    virtual void invalidateEndpoint() =0;

    virtual ~DataReceiverEndpoint(){
        if(uniqueEndpointAioPointer != nullptr) {
            nng_aio_wait(uniqueEndpointAioPointer);
            nng_aio_free(uniqueEndpointAioPointer);
        }
    }

    /**
     * Set the timeout of the function for its receive functions. This applies to both Blocking and Async receives.
     * @param ms_time - The time in milliseconds we want to set, note that -1 will give INFINITE TIME
     */
    void setReceiveTimeout(int ms_time);
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
