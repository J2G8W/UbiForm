#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "PublisherEndpoint.h"

void PublisherEndpoint::listenForConnection(const char *base, int port) {
    int rv = listenForConnectionWithRV(base, 0);
    if (rv != 0) {
        throw NngError(rv, "Publisher listen at " + std::string(base));
    }
}

int PublisherEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv;
    std::string addr = std::string(base) + ":" + std::to_string(port);
    if ((rv = nng_listen(*senderSocket, addr.c_str(), nullptr, 0)) != 0) {
        return rv;
    }
    this->listenPort = port;
    return rv;
}

// Destructor waits a short time before closing socket such that any unsent messages are released
PublisherEndpoint::~PublisherEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr && DataSenderEndpoint::socketOpen) {
        // Make sure that the messages are flushed
        nng_msleep(300);
        // We only have one actual socket so only need to close 1.
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        } else {
            std::cout << "Publisher socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void PublisherEndpoint::closeSocket() {
    if (nng_close(*senderSocket) == NNG_ECLOSED) {
        std::cerr << "This socket had already been closed" << std::endl;
    } else {
        std::cout << "Publisher socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
    }
    DataSenderEndpoint::socketOpen = false;
}
