#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "RequestEndpoint.h"

void RequestEndpoint::listenForConnection(const char *url) {
    throw SocketOpenError("Request socket trying to listen for connection");
}

int RequestEndpoint::listenForConnectionWithRV(const char *url) {
    throw SocketOpenError("Request socket trying to listen for connection");
}

void RequestEndpoint::dialConnection(const char *url) {
    int rv;
    if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing " + std::string(url) + " for a pair connection");
    }
    this->listenUrl = url;
    this->dialUrl = url;
}

// Destructor waits a short time before closing socket such that any unsent messages are released
RequestEndpoint::~RequestEndpoint() {
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

void RequestEndpoint::closeSocket() {
    if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
        DataReceiverEndpoint::socketOpen = false;
        DataSenderEndpoint::socketOpen = false;
    }
}
