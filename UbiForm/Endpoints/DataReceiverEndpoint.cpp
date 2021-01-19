#include "DataReceiverEndpoint.h"


// Receive a message, validate it against the socketManifest and return a pointer to the object.
// Use smart pointers to avoid complex memory management
std::unique_ptr<SocketMessage> DataReceiverEndpoint::receiveMessage() {
    if (!(endpointState == EndpointState::Dialed || endpointState == EndpointState::Listening)) {
        throw SocketOpenError("Could not receive message", socketType, endpointIdentifier);
    }
    int rv;
    char *buffer = nullptr;

    size_t sz;


    if ((rv = nng_recv(*receiverSocket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
        std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
        receiverSchema->validate(*receivedMessage);
        nng_free(buffer, sz);
        return receivedMessage;
    } else {
        throw NngError(rv, "nng_recv");
    }
}


// This is the public interface for asynchronously receiving messages
void DataReceiverEndpoint::asyncReceiveMessage(void (*callb)(SocketMessage *, void *), void *furtherUserData) {
    if (!(endpointState == EndpointState::Dialed || endpointState == EndpointState::Listening)) {
        throw SocketOpenError("Could not async-receive message, socket is closed", socketType, endpointIdentifier);
    }
    auto *asyncData = new AsyncData(callb, this->receiverSchema, furtherUserData, this);
    nng_aio_alloc(&(uniqueEndpointAioPointer), asyncCallback, asyncData);
    nng_recv_aio(*receiverSocket, uniqueEndpointAioPointer);
    // Purposely don't delete memory of asyncData as this will be used in the callback
}

// This is our explicitly defined callback, which does some processing THEN calls the input the callback
// It cannot be called outside the class
void DataReceiverEndpoint::asyncCallback(void *data) {
    auto *asyncInput = static_cast<AsyncData *>(data);
    int rv;
    if ((rv = nng_aio_result(asyncInput->owningEndpoint->uniqueEndpointAioPointer)) != 0) {
        std::cerr << "NNG async error\nError text: " << nng_strerror(rv) << std::endl;
        delete asyncInput;
        return;
    }

    // Extract the message from our AioPointer and create a SocketMessage for easy handling
    nng_msg *msgPointer = nng_aio_get_msg(asyncInput->owningEndpoint->uniqueEndpointAioPointer);
    char *receivedJSON = static_cast<char *>(nng_msg_body(msgPointer));
    std::unique_ptr<SocketMessage>receivedMessage = std::make_unique<SocketMessage>(receivedJSON);


    nng_msg_free(msgPointer);

    // If we fail, we don't retry we just display an error message and exit
    try {
        asyncInput->endpointSchema->validate(*receivedMessage);
        asyncInput->callback(receivedMessage.get(), asyncInput->furtherUserData);
    } catch (std::logic_error &e) {
        std::cerr << "Failed message receive as: " << e.what() << std::endl;
    }

    // Handle our own memory properly
    delete asyncInput;
}

void DataReceiverEndpoint::setReceiveTimeout(int ms_time) {
    int rv = nng_socket_set_ms(*receiverSocket, NNG_OPT_RECVTIMEO, ms_time);
    if (rv != 0) {
        throw NngError(rv, "Set receive timeout");
    }
}

void DataReceiverEndpoint::dialConnection(const char *url) {
    int rv;
    if ((rv = nng_dial(*receiverSocket, url, nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing " + std::string(url) + " for a pair connection");
    }
    this->dialUrl = url;
    this->endpointState = EndpointState::Dialed;
}

