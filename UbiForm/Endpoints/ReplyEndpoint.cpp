#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/Endpoints/ReplyEndpoint.h"


void ReplyEndpoint::dialConnection(const std::string &url) {
    throw EndpointOpenError("Reply endpoint is trying to dial a connection!", connectionParadigm,
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
                std::cerr << "This endpoint had already been closed" << std::endl;
            } else {
                if(VIEW_STD_OUTPUT) std::cout << "Reply endpoint " << endpointIdentifier << " closed" << std::endl;
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
            throw NngError(rv, "Open reply endpoint");
        } else {
            endpointState = EndpointState::Open;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
        // By default we have an infinite timeout
        setReceiveTimeout(-1);
    } else {
        throw EndpointOpenError("Can't open endpoint", connectionParadigm,
                                endpointIdentifier);
    }
}

void ReplyEndpoint::listenForConnection(const std::string &base, int port) {
    DataSenderEndpoint::listenForConnection(base, port);
}

int ReplyEndpoint::listenForConnectionWithRV(const std::string &base, int port) {
    int rv = DataSenderEndpoint::listenForConnectionWithRV(base, port);
    return rv;
}
