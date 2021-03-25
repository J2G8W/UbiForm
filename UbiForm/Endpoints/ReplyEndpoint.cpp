#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/Endpoints/ReplyEndpoint.h"


void ReplyEndpoint::dialConnection(const std::string &url) {
    throw SocketOpenError("Reply endpoint is trying to dial a connection!", socketType,
                          endpointIdentifier);
}


void ReplyEndpoint::closeEndpoint() {
    DataSenderEndpoint::closeEndpoint();
}

ReplyEndpoint::~ReplyEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(endpointState == EndpointState::Closed ||
              endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Reply socket " << endpointIdentifier << " closed" << std::endl;
            }
            endpointState = EndpointState::Invalid;

        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void ReplyEndpoint::openEndpoint() {
    if (endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_rep0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open reply socket");
        } else {
            endpointState = EndpointState::Open;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
        // By default we have an infinite timeout
        setReceiveTimeout(-1);
    } else {
        throw SocketOpenError("Can't open endpoint", socketType,
                              endpointIdentifier);
    }
}

void ReplyEndpoint::listenForConnection(const char *base, int port) {
    DataSenderEndpoint::listenForConnection(base, port);
}

int ReplyEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv = DataSenderEndpoint::listenForConnectionWithRV(base, port);
    return rv;
}
