#include "DataSenderEndpoint.h"

// Send the SocketMessage object on our socket after checking that our message is valid against our manifest
void DataSenderEndpoint::sendMessage(SocketMessage &s) {
    if(!socketOpen){
        throw SocketOpenError("Could not send message, socket is closed", socketType, endpointIdentifier);
    }
    int rv;
    std::string messageTextObject = s.stringify();
    // Effectively treat this as cast, as the pointer is still to stack memory
    const char *buffer = messageTextObject.c_str();

    try {
        senderSchema->validate(s);
        if ((rv = nng_send(*senderSocket, (void *) buffer, strlen(buffer) + 1, 0)) != 0) {
            throw NngError(rv, "nng_send");
        }
    } catch (std::logic_error &e){
        std::cerr << "Message couldn't send as: " << e.what() << std::endl;
    }
}

void DataSenderEndpoint::asyncSendMessage(SocketMessage &s) {
    if(!socketOpen){
        throw SocketOpenError("Could not async-send message, socket is closed", socketType, endpointIdentifier);
    }
    nng_aio_wait(nngAioPointer);
    std::string text = s.stringify();
    const char * textArray = text.c_str();

    nng_msg * msg;
    int rv;
    if ((rv = nng_msg_alloc(&msg, 0)) !=0){
        throw NngError(rv, "Allocating async message space");
    }
    if ((rv =nng_msg_append(msg, (void*) textArray, text.size()+1)) != 0){
        throw NngError(rv, "Creating Async Message");
    }
    nng_aio_set_msg(nngAioPointer, msg);

    nng_send_aio(*senderSocket, nngAioPointer);

}

void DataSenderEndpoint::asyncCleanup(void * data) {
    auto * asyncInput = static_cast<DataSenderEndpoint *>(data);

    if (nng_aio_result(asyncInput->nngAioPointer) != 0){
        // Failed message send, we do cleanup
        asyncInput->numSendFails ++;
        nng_msg * msg = nng_aio_get_msg(asyncInput->nngAioPointer);
        nng_msg_free(msg);
    }
}

void DataSenderEndpoint::setSendTimeout(int ms_time) {
    int rv = nng_socket_set_ms(*senderSocket, NNG_OPT_SENDTIMEO,ms_time);
    if(rv != 0){
        throw NngError(rv,"Set send timeout");
    }
}
