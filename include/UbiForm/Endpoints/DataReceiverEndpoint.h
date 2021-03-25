#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include <thread>
#include "../SocketMessage.h"
#include "../SchemaRepresentation/EndpointSchema.h"
#include "Endpoint.h"

/**
 * This class represents a data receiver in our design. I have made the choice that it is only able to dial, not listen
 * as this fits with the IOT scope best. The receive of messages is the same no matter the type of endpoint BUT our dialing
 * process is different, hence why this is extended in the child classes
 */
class DataReceiverEndpoint : virtual public Endpoint {
private:

    static void asyncCallback(void *data);

    // Never freed...
    nng_aio *uniqueEndpointAioPointer = nullptr;

    /**
     * This is the data structure used only internally for our async functions, it can have arbitrary data in it with
     * the void*.
     */
    struct AsyncData {
        receiveMessageCallBack callback;

        std::shared_ptr<EndpointSchema> endpointSchema;
        DataReceiverEndpoint *owningEndpoint;
        void *furtherUserData;

        AsyncData(receiveMessageCallBack cb, std::shared_ptr<EndpointSchema> endpointSchema,
                  void *furtherUserData, DataReceiverEndpoint *dataReceiverEndpoint) :
                callback(cb), endpointSchema(endpointSchema) {

            this->owningEndpoint = dataReceiverEndpoint;
            this->furtherUserData = furtherUserData;
        };
    };

protected:
    // These are all used by child classes in reporting errors


    // Socket is initialised in extending class
    nng_socket *receiverSocket = nullptr;
    // Schema is shared with the parent that houses this endpoint
    std::shared_ptr<EndpointSchema> receiverSchema;


    std::string dialUrl = "";

    std::unique_ptr<SocketMessage> rawReceiveMessage();

public:
    explicit DataReceiverEndpoint(std::shared_ptr<EndpointSchema> &es) {
        receiverSchema = es;
    };

    /**
    * Extended by child classes, to dial external connections
    * @param url
     * @throws NngError when we are unable to dial the url properly
    */
    virtual void dialConnection(const std::string &url);

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
     * @param callback - The function which is called when a SocketMessage is received - DON'T BE A BLOCKING FUNCTION or we
     * can get deadlock scenarios
     * @param furtherUserData - Extra data which you want to be available in the call back
     * @throws SocketOpenError - When the socket is not open
     */
    void asyncReceiveMessage(receiveMessageCallBack callback, void *furtherUserData);

    /**
     * Get the URL we are currently dialled onto
     * @return dialUrl
     */
    std::string getDialUrl() { return dialUrl; }



    /**
     * This function closes our socket. If extended it should call the parent then handle states of other things.
     * Note that a closed socket must be re-opened before being use for dialing etc
     */
    virtual void closeEndpoint() ;

    /**
     * Open a socket ready for it dial someone
     */
    virtual void openEndpoint() = 0;


    virtual ~DataReceiverEndpoint() {
        if (uniqueEndpointAioPointer != nullptr) {
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
