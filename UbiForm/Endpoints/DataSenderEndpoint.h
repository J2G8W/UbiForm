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
    SocketType endpointType;
    // Socket is initialised in extending class
    nng_socket * senderSocket = nullptr;
    std::shared_ptr<EndpointSchema>senderSchema;
    std::string listenUrl;
    bool socketOpen = false;
public:
    explicit DataSenderEndpoint( std::shared_ptr<EndpointSchema>& es, const std::string & endpointIdentifier, SocketType endpointType):
        endpointIdentifier(endpointIdentifier), endpointType(endpointType), nngAioPointer(){
        senderSchema = es;
        nng_aio_alloc(&(this->nngAioPointer), asyncCleanup, this);
        nng_aio_set_timeout(nngAioPointer, 50);
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void listenForConnection(const char *url) = 0;
    virtual int listenForConnectionWithRV(const char *url) = 0;


    void sendMessage(SocketMessage &s);
    void asyncSendMessage(SocketMessage &s);
    std::string getListenUrl(){return listenUrl;}

    virtual void closeSocket() = 0;
    virtual ~DataSenderEndpoint(){
        nng_aio_wait(nngAioPointer);
        nng_aio_free(nngAioPointer);
    }

};


#endif //UBIFORM_DATASENDERENDPOINT_H
