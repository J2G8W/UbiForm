#include "DataSenderEndpoint.h"

// Send the SocketMessage object on our socket after checking that our message is valid against our manifest
void DataSenderEndpoint::sendMessage(SocketMessage &s) {
    int rv;
    std::string messageTextObject = s.stringify();
    // Effectively treat this as cast, as the pointer is still to stack memory
    const char *buffer = messageTextObject.c_str();

    try {
        senderSchema.validate(s);
        if ((rv = nng_send(*senderSocket, (void *) buffer, strlen(buffer) + 1, 0)) != 0) {
            fatal("nng_send (msg)", rv);
        }
    } catch (std::logic_error &e){
        std::cerr << "Message couldn't send as: " << e.what();
    }
}