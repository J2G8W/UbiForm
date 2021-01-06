#include <string>

#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>


#include "../Utilities/UtilityFunctions.h"
#include "PairEndpoint.h"


// Note that outgoing means it dials an external URL
void PairEndpoint::dialConnection(const char *url) {
    int rv;
    if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing " + std::string(url) + " for a pair connection");
    }
    this->listenUrl = url;
    this->dialUrl = url;
}

// Incoming means it will listen on an internal URL
void PairEndpoint::listenForConnection(const char *url){
    int rv = listenForConnectionWithRV(url);
    if (rv != 0){
        throw NngError(rv,"Listening on " + std::string(url));
    }
}
int PairEndpoint::listenForConnectionWithRV(const char *url) {
    int rv;
    if((rv = nng_listen(*senderSocket, url, nullptr, 0)) != 0) {
        return rv;
    }
    this->listenUrl = url;
    this->dialUrl = url;
    return rv;
}


// Destructor waits a short time before closing socket such that any unsent messages are released
PairEndpoint::~PairEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr && DataSenderEndpoint::socketOpen && DataReceiverEndpoint::socketOpen) {
        // Make sure that the messages are flushed
        nng_msleep(300);
        // We only have one actual socket so only need to close 1.
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void PairEndpoint::closeSocket() {
    if (DataReceiverEndpoint::socketOpen && DataSenderEndpoint::socketOpen) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }
        DataReceiverEndpoint::socketOpen = false;
        DataSenderEndpoint::socketOpen = false;
    }
}
