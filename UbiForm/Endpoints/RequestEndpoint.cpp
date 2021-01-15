#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "RequestEndpoint.h"

void RequestEndpoint::listenForConnection(const char *base, int port) {
    throw SocketOpenError("Request socket trying to listen for connection",
                          DataSenderEndpoint::socketType, DataSenderEndpoint::endpointIdentifier);
}

int RequestEndpoint::listenForConnectionWithRV(const char *base, int port) {
    throw SocketOpenError("Request socket trying to listen for connection",
                          DataSenderEndpoint::socketType, DataSenderEndpoint::endpointIdentifier);
}

void RequestEndpoint::dialConnection(const char *url) {
    int rv;
    if(dialUrl != std::string(url)) {
        // Before dialling a new location we close the old socket (means same endpoint can be reused)
        if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
            nng_msleep(300);
            nng_close(*senderSocket);
            DataReceiverEndpoint::socketOpen = false;
            DataSenderEndpoint::socketOpen = false;
        }

        if ((rv = nng_req0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open RD connection request socket");
        } else {
            DataReceiverEndpoint::socketOpen = true;
            DataSenderEndpoint::socketOpen = true;
            setReceiveTimeout(500);
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
        if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
            throw NngError(rv, "Dialing " + std::string(url) + " for a request connection");
        }
        this->dialUrl = url;
    }
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
        }else{
            std::cout << "Request socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
        }

    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void RequestEndpoint::closeSocket() {
    if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }else{
            std::cout << "Request socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
        }
        DataReceiverEndpoint::socketOpen = false;
        DataSenderEndpoint::socketOpen = false;
    }
}
