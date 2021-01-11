#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "ReplyEndpoint.h"

void ReplyEndpoint::listenForConnection(const char *base, int port) {
    int rv = listenForConnectionWithRV(base, port);
    if (rv != 0){
        throw NngError(rv,"Listening on " + std::string(base));
    }
}

int ReplyEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv;
    nng_listener l;
    std::string addr = std::string(base) + ":" + std::to_string(port);
    if((rv = nng_listen(*senderSocket, addr.c_str(), &l, 0)) != 0) {
        return rv;
    }
    this->port = port;
    this->dialUrl = base;
    return rv;
}

void ReplyEndpoint::dialConnection(const char *url) {
    throw SocketOpenError("Reply endpoint is trying to dial a connection!", DataSenderEndpoint::endpointType, DataSenderEndpoint::endpointIdentifier);
}

void ReplyEndpoint::setTimeout(int timeout) {
    nng_socket_set_ms(*senderSocket, NNG_OPT_RECVTIMEO,timeout);
}

void ReplyEndpoint::closeSocket() {
    if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }else{
            std::cout << "Reply socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
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
        }else{
            std::cout << "Reply socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}
