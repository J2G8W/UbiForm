#include <string>

#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>


#include "../Utilities/UtilityFunctions.h"
#include "../../include/UbiForm/Endpoints/PairEndpoint.h"


// Destructor waits a short time before closing socket such that any unsent messages are released
PairEndpoint::~PairEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(DataReceiverEndpoint::endpointState == EndpointState::Closed ||
             DataReceiverEndpoint::endpointState == EndpointState::Invalid) &&
            (DataSenderEndpoint::endpointState == EndpointState::Closed ||
             DataSenderEndpoint::endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Pair socket " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
            }
            DataSenderEndpoint::endpointState = EndpointState::Invalid;
            DataReceiverEndpoint::endpointState = EndpointState::Invalid;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void PairEndpoint::closeEndpoint() {
    DataSenderEndpoint::closeEndpoint();
    if (DataReceiverEndpoint::endpointState != EndpointState::Invalid) {
        DataReceiverEndpoint::endpointState = EndpointState::Closed;
    }
}

void PairEndpoint::openEndpoint() {
    if (DataReceiverEndpoint::endpointState == EndpointState::Closed &&
        DataSenderEndpoint::endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_pair0_open(senderSocket)) != 0) {
            throw NngError(rv, "Making pair connection");
        } else {
            DataSenderEndpoint::endpointState = EndpointState::Open;
            DataReceiverEndpoint::endpointState = EndpointState::Open;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
    } else {
        throw SocketOpenError("Can't open endpoint", DataSenderEndpoint::socketType,
                              DataSenderEndpoint::endpointIdentifier);
    }
}

void PairEndpoint::listenForConnection(const char *base, int port) {
    DataSenderEndpoint::listenForConnection(base, port);
}

int PairEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv = DataSenderEndpoint::listenForConnectionWithRV(base, port);
    if (rv == 0) {
        DataReceiverEndpoint::endpointState = EndpointState::Listening;
    }
    return rv;
}

void PairEndpoint::dialConnection(const char *url) {
    DataReceiverEndpoint::dialConnection(url);
    DataSenderEndpoint::endpointState = EndpointState::Dialed;
}

