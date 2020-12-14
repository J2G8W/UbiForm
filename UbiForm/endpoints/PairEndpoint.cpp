#include <string>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>


#include "../general_functions.h"
#include "PairEndpoint.h"


// TODO - error handling currently very fatal on NNG's end

// Note that outgoing means it dials an external URL
void PairEndpoint::dialConnection(const char *url) {
    int rv;
    if ((rv = nng_pair0_open(senderSocket)) != 0) {
        fatal("nng_pair0_open", rv);
    }else{
        socketOpen = true;
    }
    // Use the same socket for sending and receiving
    receiverSocket = senderSocket;
    if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
        std::ostringstream error;
        error << "PairEndpoint error dialing up: " << url << std::endl;
        error << "NNG error code " << rv << " type - " << nng_strerror(rv);
        throw std::logic_error(error.str());
    }

}

// Incoming means it will listen on an internal URL
void PairEndpoint::listenForConnection(const char *url){
    int rv;
    if ((rv = nng_pair0_open(senderSocket)) != 0) {
        fatal("nng_pair0_open", rv);
    }else{
        socketOpen = true;
    }
    // Use the same socket for sending and receiving
    receiverSocket = senderSocket;

    if ((rv = nng_listen(*senderSocket, url, nullptr, 0)) != 0) {
        std::ostringstream error;
        error << "PairEndpoint error listening on: " << url << std::endl;
        error << "NNG error code " << rv << " type - " << nng_strerror(rv);
        throw std::logic_error(error.str());
    }

}

// Destructor waits a short time before closing socket such that any unsent messages are released
PairEndpoint::~PairEndpoint() {
    int rv;
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr && socketOpen) {
        // Make sure that the messages are flushed
        sleep(1);
        // We only have one actual socket so only need to close 1.
        if ((rv = nng_close(*senderSocket)) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }

    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}
