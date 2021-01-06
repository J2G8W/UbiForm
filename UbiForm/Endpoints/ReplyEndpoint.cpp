#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "ReplyEndpoint.h"

void ReplyEndpoint::listenForConnection(const char *url) {
    int rv = listenForConnectionWithRV(url);
    if (rv != 0){
        throw NngError(rv,"Listening on " + std::string(url));
    }
}

int ReplyEndpoint::listenForConnectionWithRV(const char *url) {
    int rv;
    if((rv = nng_listen(*senderSocket, url, nullptr, 0)) != 0) {
        return rv;
    }
    this->listenUrl = url;
    this->dialUrl = url;
    return rv;
}

void ReplyEndpoint::dialConnection(const char *url) {
    throw SocketOpenError("Reply endpoint is trying to dial a connection!");
}

void ReplyEndpoint::closeSocket() {
    if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
        DataReceiverEndpoint::socketOpen = false;
        DataSenderEndpoint::socketOpen = false;
    }
}

ReplyEndpoint::~ReplyEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr && DataSenderEndpoint::socketOpen && DataReceiverEndpoint::socketOpen) {
        // Make sure that the messages are flushed
        nng_msleep(300);
        // We only have one actual socket so only need to close 1.
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}