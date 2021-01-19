#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H

#include <memory>

#include <nng/nng.h>
#include "../SocketMessage.h"

#include "../SchemaRepresentation/EndpointSchema.h"

/**
 * Class used to represent our endpoints which send data.
 */
class DataSenderEndpoint {
private:
    /**
     * This is called once we are happy the message has been properly sent, and we cleanup the mess
     */
    static void asyncCleanup(void *);

    int numSendFails = 0;


    nng_aio *nngAioPointer;
protected:
    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;

    // Socket is initialised in extending class
    nng_socket *senderSocket = nullptr;
    std::shared_ptr<EndpointSchema> senderSchema;
    int listenPort = -1;

    EndpointState endpointState = EndpointState::Closed;
public:
    explicit DataSenderEndpoint(std::shared_ptr<EndpointSchema> &es, const std::string &endpointIdentifier,
                                SocketType socketType, const std::string &endpointType) :
            endpointIdentifier(endpointIdentifier), socketType(socketType), nngAioPointer(),
            endpointType(endpointType) {
        senderSchema = es;
        nng_aio_alloc(&(this->nngAioPointer), asyncCleanup, this);
        nng_aio_set_timeout(nngAioPointer, 50);
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void listenForConnection(const char *base, int port);

    virtual int listenForConnectionWithRV(const char *base, int port);

    /**
     * Blocking send on our Socket
     * @param s - Message to send
     * @throws NngError - When there is underlying socket issues
     * @throws SocketOpenError - When the socket has already been closed
     */
    void sendMessage(SocketMessage &s);

    /**
     * Non blocking send on our socket
     * @param s - Message to send
     * @throws NngError - When we run out of space or something. Won't error on a send error (just throws it away)
     */
    void asyncSendMessage(SocketMessage &s);

    int getListenPort() { return listenPort; }

    std::string &getSenderEndpointID() { return endpointIdentifier; }

    std::string &getSenderEndpointType() { return endpointType; }

    EndpointState getSenderState(){return endpointState;}

    virtual void openEndpoint() = 0;

    virtual void closeEndpoint();

    virtual void invalidateEndpoint() = 0;

    virtual ~DataSenderEndpoint() {
        nng_aio_wait(nngAioPointer);
        nng_aio_free(nngAioPointer);
    }

    /**
     * Set timeout for our sending (only blocking send)
     * @param ms_time - Milliseconds to wait before erroring out. (-1 means inifinite time)
     */
    void setSendTimeout(int ms_time);
};


#endif //UBIFORM_DATASENDERENDPOINT_H
