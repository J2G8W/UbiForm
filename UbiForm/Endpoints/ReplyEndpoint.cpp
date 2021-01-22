#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/Endpoints/ReplyEndpoint.h"


void ReplyEndpoint::dialConnection(const char *url) {
    throw SocketOpenError("Reply endpoint is trying to dial a connection!", DataSenderEndpoint::socketType,
                          DataSenderEndpoint::endpointIdentifier);
}


void ReplyEndpoint::closeEndpoint() {
    DataSenderEndpoint::closeEndpoint();
    if (DataReceiverEndpoint::endpointState != EndpointState::Invalid) {
        DataReceiverEndpoint::endpointState = EndpointState::Closed;
    }
}

ReplyEndpoint::~ReplyEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(DataReceiverEndpoint::endpointState == EndpointState::Closed ||
              DataReceiverEndpoint::endpointState == EndpointState::Invalid) &&
            !(DataSenderEndpoint::endpointState == EndpointState::Closed ||
             DataSenderEndpoint::endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Reply socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
            }
            DataSenderEndpoint::endpointState = EndpointState::Invalid;
            DataReceiverEndpoint::endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void ReplyEndpoint::openEndpoint() {
    if (DataSenderEndpoint::endpointState == EndpointState::Closed &&
        DataReceiverEndpoint::endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_rep0_open(senderSocket)) != 0) {
            throw NngError(rv, "Open reply socket");
        } else {
            DataReceiverEndpoint::endpointState = EndpointState::Open;
            DataSenderEndpoint::endpointState = EndpointState::Open;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
        // By default we have an infinite timeout
        setReceiveTimeout(-1);
    } else {
        throw SocketOpenError("Can't open endpoint", DataSenderEndpoint::socketType,
                              DataSenderEndpoint::endpointIdentifier);
    }
}

void ReplyEndpoint::listenForConnection(const char *base, int port) {
    DataSenderEndpoint::listenForConnection(base, port);
}

int ReplyEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv = DataSenderEndpoint::listenForConnectionWithRV(base, port);
    if (rv == 0) {
        DataReceiverEndpoint::endpointState = EndpointState::Listening;
    }
    return rv;
}
