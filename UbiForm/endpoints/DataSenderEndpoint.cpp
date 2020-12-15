#include "DataSenderEndpoint.h"
#include "../general_functions.h"

// Send the SocketMessage object on our socket after checking that our message is valid against our manifest
void DataSenderEndpoint::sendMessage(SocketMessage &s) {
    int rv;
    std::string messageTextObject = s.stringify();
    // Effectively treat this as cast, as the pointer is still to stack memory
    const char *buffer = messageTextObject.c_str();

    try {
        senderSchema->validate(s);
        if ((rv = nng_send(*senderSocket, (void *) buffer, strlen(buffer) + 1, 0)) != 0) {
            throw NNG_error(rv, "nng_send");
        }
    } catch (std::logic_error &e){
        std::cerr << "Message couldn't send as: " << e.what() << std::endl;
    }
}

void DataSenderEndpoint::asyncSendMessage(SocketMessage &s) {
    auto *ad = new AsyncData(AsyncCleanup);

    std::string text = s.stringify();
    const char * textArray = text.c_str();

    nng_msg * msg;
    int rv;
    if ((rv = nng_msg_alloc(&msg, 0)) !=0){
        fatal("nng_msg_alloc",rv);
    }
    if ((rv =nng_msg_append(msg, (void*) textArray, text.size()+1)) != 0){
        fatal("nng_msg_append", rv);
    }
    nng_aio_set_msg(ad->nngAioPointer, msg);
    nng_send_aio(*senderSocket, ad->nngAioPointer);

}

void DataSenderEndpoint::AsyncCleanup(void * data) {
    auto * asyncInput = static_cast<AsyncData *>(data);
    int rv;

    if ((rv = nng_aio_result(asyncInput->nngAioPointer)) != 0){
        // Failed message send, we do cleanup
        nng_msg * msg = nng_aio_get_msg(asyncInput->nngAioPointer);
        nng_msg_free(msg);
    }
    delete asyncInput;
}
