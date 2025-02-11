#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/Endpoints/PublisherEndpoint.h"

void PublisherEndpoint::openEndpoint() {
    if (endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_pub0_open(senderSocket)) != 0) {
            throw NngError(rv, "Opening of publisher endpoint");
        } else {
            endpointState = EndpointState::Open;
        }
    } else {
        throw EndpointOpenError("Can't open endpoint", connectionParadigm,
                                endpointIdentifier);
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
PublisherEndpoint::~PublisherEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(endpointState == EndpointState::Closed ||
            endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This endpoint had already been closed" << std::endl;
            } else {
                if(VIEW_STD_OUTPUT) std::cout << "Publisher endpoint " << endpointIdentifier << " closed" << std::endl;
            }
            endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

