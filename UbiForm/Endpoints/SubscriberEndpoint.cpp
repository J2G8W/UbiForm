#include "../../include/UbiForm/Endpoints/SubscriberEndpoint.h"

#include <nng/protocol/pubsub0/sub.h>
#include <nng/supplemental/util/platform.h>


void SubscriberEndpoint::dialConnection(const char *url) {
    DataReceiverEndpoint::dialConnection(url);

    int rv;
    // Default is that we subscribe to all topics
    if ((rv = nng_socket_set(*receiverSocket, NNG_OPT_SUB_SUBSCRIBE, nullptr, 0)) != 0) {
        throw NngError(rv, "Setting subscribe topics to ALL");
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
SubscriberEndpoint::~SubscriberEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (receiverSocket != nullptr) {
        if (DataReceiverEndpoint::endpointState == EndpointState::Dialed ||
            DataReceiverEndpoint::endpointState == EndpointState::Listening ||
            DataReceiverEndpoint::endpointState == EndpointState::Open) {
            nng_msleep(300);
            if (nng_close(*receiverSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Subscriber socket " << DataReceiverEndpoint::endpointIdentifier << " closed" << std::endl;
            }
            DataReceiverEndpoint::endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete receiverSocket;
}

void SubscriberEndpoint::closeEndpoint() {
    if (nng_close(*receiverSocket) == NNG_ECLOSED) {
        std::cerr << "This socket had already been closed" << std::endl;
    } else {
        std::cout << "Subscriber socket " << DataReceiverEndpoint::endpointIdentifier << " closed" << std::endl;
    }
    DataReceiverEndpoint::endpointState = EndpointState::Closed;
}

void SubscriberEndpoint::openEndpoint() {
    if (DataReceiverEndpoint::endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_sub0_open(receiverSocket)) != 0) {
            throw NngError(rv, "Opening subscriber socket");
        } else {
            DataReceiverEndpoint::endpointState = EndpointState::Open;
        }
    } else {
        throw SocketOpenError("Can't open endpoint", DataReceiverEndpoint::socketType,
                              DataReceiverEndpoint::endpointIdentifier);
    }
}
