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
    if (dialUrl != std::string(url)) {
        // Before dialling a new location we close the old socket (means same endpoint can be reused)
        if (DataReceiverEndpoint::endpointState == EndpointState::Dialed ||
            DataReceiverEndpoint::endpointState == EndpointState::Listening) {
            closeSocket();
        }
        if (DataReceiverEndpoint::endpointState == EndpointState::Closed) {
            openEndpoint();
        }

        if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
            throw NngError(rv, "Dialing " + std::string(url) + " for a request connection");
        }
        this->dialUrl = url;
        this->listenPort = -1;
        DataReceiverEndpoint::endpointState = EndpointState::Dialed;
        DataSenderEndpoint::endpointState = EndpointState::Dialed;
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
RequestEndpoint::~RequestEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if ((DataReceiverEndpoint::endpointState == EndpointState::Dialed ||
             DataReceiverEndpoint::endpointState == EndpointState::Listening ||
             DataReceiverEndpoint::endpointState == EndpointState::Open) &&
            (DataSenderEndpoint::endpointState == EndpointState::Dialed ||
             DataSenderEndpoint::endpointState == EndpointState::Listening ||
             DataSenderEndpoint::endpointState == EndpointState::Open)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Request socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
            }
            DataSenderEndpoint::endpointState = EndpointState::Invalid;
            DataReceiverEndpoint::endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void RequestEndpoint::closeSocket() {
    DataSenderEndpoint::closeSocket();
    if (DataReceiverEndpoint::endpointState != EndpointState::Invalid) {
        DataReceiverEndpoint::endpointState = EndpointState::Closed;
    }
}

void RequestEndpoint::openEndpoint() {
    int rv;
    if (DataReceiverEndpoint::endpointState == EndpointState::Closed &&
        DataSenderEndpoint::endpointState == EndpointState::Closed) {
        if ((rv = nng_req0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open RD connection request socket");
        } else {
            DataReceiverEndpoint::endpointState = EndpointState::Open;
            DataSenderEndpoint::endpointState = EndpointState::Open;
            // Use the same socket for sending and receiving
            receiverSocket = senderSocket;
            // Set timeout to a reasonably small value
            setReceiveTimeout(500);
        }
    } else {
        throw SocketOpenError("Can't open endpoint", DataSenderEndpoint::socketType,
                              DataSenderEndpoint::endpointIdentifier);
    }
}
