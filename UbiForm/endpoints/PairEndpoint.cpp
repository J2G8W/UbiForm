#include <string>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>


#include "../general_functions.h"
#include "PairEndpoint.h"


// TODO - error handling currently very fatal on NNG's end

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
        throw NngError(rv,"Listening on " std::string(url));
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

