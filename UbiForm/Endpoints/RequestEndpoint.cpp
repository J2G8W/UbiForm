#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/Endpoints/RequestEndpoint.h"

void RequestEndpoint::listenForConnection(const char *base, int port) {
    throw SocketOpenError("Request socket trying to listen for connection",
                          socketType, endpointIdentifier);
}

int RequestEndpoint::listenForConnectionWithRV(const char *base, int port) {
    throw SocketOpenError("Request socket trying to listen for connection",
                          socketType, endpointIdentifier);
}

void RequestEndpoint::dialConnection(const std::string &url) {
    int rv;
    if (dialUrl != std::string(url)) {
        // Before dialling a new location we close the old socket (means same endpoint can be reused)
        if (endpointState == EndpointState::Dialed ||
            endpointState == EndpointState::Listening ) {
            closeEndpoint();
        }
        if (endpointState == EndpointState::Closed) {
            openEndpoint();
        }

        if ((rv = nng_dial(*senderSocket, url.c_str(), nullptr, 0)) != 0) {
            throw NngError(rv, "Dialing " + std::string(url) + " for a request connection");
        }
        this->dialUrl = url;
        this->listenPort = -1;
        endpointState = EndpointState::Dialed;
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
RequestEndpoint::~RequestEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(endpointState == EndpointState::Closed ||
              endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Request socket " << endpointIdentifier << " closed" << std::endl;
            }
            endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void RequestEndpoint::closeEndpoint() {
    DataSenderEndpoint::closeEndpoint();
}

void RequestEndpoint::openEndpoint() {
    int rv;
    if (endpointState == EndpointState::Closed) {
        if ((rv = nng_req0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open RD connection request socket");
        } else {
            endpointState = EndpointState::Open;
            // Use the same socket for sending and receiving
            receiverSocket = senderSocket;
            // Set timeout to a reasonably small value
            setReceiveTimeout(500);
        }
    } else {
        throw SocketOpenError("Can't open endpoint", socketType,
                              endpointIdentifier);
    }
}
