#include "DataReceiverEndpoint.h"
#include <nng/nng.h>


// Receive a message, validate it against the socketManifest and return a pointer to the object.
// Use smart pointers to avoid complex memory management
std::unique_ptr<SocketMessage> DataReceiverEndpoint::receiveMessage() {
    int rv;
    char *buffer = nullptr;

    size_t sz;

    // Is effectively a blocking receive which waits until the correct information comes in until it returns
    while (true) {
        try{
            if ((rv = nng_recv(*receiverSocket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
                std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
                receiverSchema->validate(*receivedMessage);
                nng_free(buffer, sz);
                return receivedMessage;
            }else{
                throw NNG_error(rv, "nng_recv");
            }
        } catch(std::logic_error &e) {
            std::cerr << "Failed message receive send as: " << e.what() << std::endl;
        }
    }
}



// This is the public interface for asynchronously receiving messages
// TODO - improve type safety here - templates rather than void*
void DataReceiverEndpoint::asyncReceiveMessage(void (*callb)(SocketMessage *, void *), void *furtherUserData) {
    auto *asyncData = new AsyncData(callb, this->receiverSchema, furtherUserData);

    nng_recv_aio(*receiverSocket, asyncData->nngAioPointer);
    // Purposely don't delete memory of asyncData as this will be used in the callback
}

// This is our explicitly defined callback, which does some processing THEN calls the input the callback
// It cannot be called outside the class
void DataReceiverEndpoint::asyncCallback(void *data) {
    auto * asyncInput = static_cast<AsyncData *>(data);

    int rv;
    if ((rv = nng_aio_result(asyncInput->nngAioPointer)) != 0) {
        std::cerr << "NNG async error\nError text: " << nng_strerror(rv) << std::endl;
        delete asyncInput;
        return;
    }

    // Extract the message from our AioPointer and create a SocketMessage for easy handling
    nng_msg * msgPointer = nng_aio_get_msg(asyncInput->nngAioPointer);
    char* receivedJSON = static_cast<char *>(nng_msg_body(msgPointer));
    auto *receivedMessage = new SocketMessage(receivedJSON);

    nng_msg_free(msgPointer);

    // If we fail, we don't retry we just display an error message and exit
    try {
        asyncInput->endpointSchema->validate(*receivedMessage);
        asyncInput->callback(receivedMessage,asyncInput->furtherUserData);
    }catch(std::logic_error &e) {
        std::cerr << "Failed message receive send as: " << e.what() << std::endl;
    }
    // Handle our own memory properly
    delete receivedMessage;
    delete asyncInput;
}

