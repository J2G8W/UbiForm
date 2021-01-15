#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H

#include <memory>

#include <nng/nng.h>
#include "../SocketMessage.h"

#include "../SchemaRepresentation/EndpointSchema.h"

class DataSenderEndpoint {
private:
    static void asyncCleanup(void*);
    int numSendFails = 0;


    nng_aio *nngAioPointer;
protected:
    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;

    // Socket is initialised in extending class
    nng_socket * senderSocket = nullptr;
    std::shared_ptr<EndpointSchema>senderSchema;
    bool socketOpen = false;
    int port = -1;
public:
    explicit DataSenderEndpoint( std::shared_ptr<EndpointSchema>& es, const std::string & endpointIdentifier,
                                 SocketType socketType, const std::string& endpointType):
        endpointIdentifier(endpointIdentifier), socketType(socketType), nngAioPointer(), endpointType(endpointType){
        senderSchema = es;
        nng_aio_alloc(&(this->nngAioPointer), asyncCleanup, this);
        nng_aio_set_timeout(nngAioPointer, 50);
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void listenForConnection(const char *base, int port) = 0;
    virtual int listenForConnectionWithRV(const char *base, int port) = 0;


    void sendMessage(SocketMessage &s);
    void asyncSendMessage(SocketMessage &s);
    int getListenPort(){return port;}

    std::string& getSenderEndpointID(){return endpointIdentifier;}
    std::string& getSenderEndpointType(){return endpointType;}

    virtual void closeSocket() = 0;
    virtual ~DataSenderEndpoint(){
        nng_aio_wait(nngAioPointer);
        nng_aio_free(nngAioPointer);
    }

};


#endif //UBIFORM_DATASENDERENDPOINT_H
