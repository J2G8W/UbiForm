#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H

#include <memory>

#include <nng/nng.h>
#include <thread>
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

    void rawSendMessage(SocketMessage& sm);

public:
    explicit DataSenderEndpoint(std::shared_ptr<EndpointSchema> &es, const std::string &endpointIdentifier,
                                SocketType socketType, const std::string &endpointType) :
            endpointIdentifier(endpointIdentifier), socketType(socketType), nngAioPointer(),
            endpointType(endpointType) {
        senderSchema = es;
        nng_aio_alloc(&(this->nngAioPointer), asyncCleanup, this);
        nng_aio_set_timeout(nngAioPointer, 50);
    };

    /**
     * We listen for a connection to us
     * @param base - The base address we want to listen on (e.g. tcp://127.0.0.1). Often we use "tcp:// *" (without the space)
     * @param port - The port to listen on
     * @throws NngError - when we are unable to listen on that address for whatever reason
     */
    virtual void listenForConnection(const char *base, int port);

    /**
     * Listen for a connection but don't throw any errors and instead return a return variable to do things with. Should
     * not be used unless you know what to do (use normal listenForConnection)
     * @param base - The base address we want to listen on (e.g. tcp://127.0.0.1). Often we use "tcp:// *" (without the space)
     * @param port - The port to listen on
     * @return A return variable representing how the listening has gone
     */
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

    SocketType getSenderSocketType(){return socketType;}

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
