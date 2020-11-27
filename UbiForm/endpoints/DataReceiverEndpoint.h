#ifndef UBIFORM_DATARECEIVERENDPOINT_H
#define UBIFORM_DATARECEIVERENDPOINT_H


#include <memory>
#include <utility>
#include <nng/nng.h>
#include "../SocketMessage.h"
#include "EndpointSchema.h"

class DataReceiverEndpoint {
private:
    static void asyncCallback(void* data);

    // This is the data structure which we will pass into our callback
    // Note this is not part of any class, it is hidden
    struct AsyncData{
        // The nngAioPointer has to be initialised by C style semantics
        nng_aio *nngAioPointer;
        void (*callback)(SocketMessage *);
        std::shared_ptr<EndpointSchema> endpointSchema;

        AsyncData(void (*cb)(SocketMessage *), std::shared_ptr<EndpointSchema> es) : callback(cb), endpointSchema(es){
            // So we allocate the async_io with a pointer to our asyncCallback and a pointer to this object
            nng_aio_alloc(&(this->nngAioPointer), asyncCallback, this);
        };

        ~AsyncData(){
            nng_aio_free(nngAioPointer);
            // Rest of data handled automatically
        }
    };
protected:
    // Socket is initialised in extending class
    nng_socket * receiverSocket = nullptr;
    // Schema is shared with the parent that houses this endpoint
    std::shared_ptr<EndpointSchema>receiverSchema;
public:
    explicit DataReceiverEndpoint( std::shared_ptr<EndpointSchema>& es){
        receiverSchema = es;
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void dialConnection(const char *url) = 0;


    std::unique_ptr<SocketMessage> receiveMessage();

    // Here we receive a message asynchronously, it accepts a function which does some work on a SocketMessage
    // NOTE that the SocketMessage should not be freed by the given func (library handles the memory management)
    void asyncReceiveMessage(void (*callb)(SocketMessage *));
};


#endif //UBIFORM_DATARECEIVERENDPOINT_H
