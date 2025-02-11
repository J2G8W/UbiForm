#include "../../include/UbiForm/Endpoints/SubscriberEndpoint.h"

#include <nng/protocol/pubsub0/sub.h>
#include <nng/supplemental/util/platform.h>


void SubscriberEndpoint::dialConnection(const std::string &url) {
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
        if (!(endpointState == EndpointState::Closed ||
            endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*receiverSocket) == NNG_ECLOSED) {
                std::cerr << "This endpoint had already been closed" << std::endl;
            } else {
                if(VIEW_STD_OUTPUT) std::cout << "Subscriber endpoint " << endpointIdentifier << " closed" << std::endl;
            }
            endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete receiverSocket;
}


void SubscriberEndpoint::openEndpoint() {
    if (endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_sub0_open(receiverSocket)) != 0) {
            throw NngError(rv, "Opening subscriber endpoint");
        } else {
            endpointState = EndpointState::Open;
        }
    } else {
        throw EndpointOpenError("Can't open endpoint", connectionParadigm,
                                endpointIdentifier);
    }
}
