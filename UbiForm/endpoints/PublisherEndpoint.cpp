#include <unistd.h>
#include "PublisherEndpoint.h"

void PublisherEndpoint::listenForConnection(const char *url) {
    int rv;
    if ((rv = nng_pub0_open(senderSocket)) != 0) {
        fatal("nng_pub0_open", rv);
    }else{
        socketOpen = true;
    }

    if ((rv = nng_listen(*senderSocket, url, nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    this->listenUrl = url;
}

// Destructor waits a short time before closing socket such that any unsent messages are released
PublisherEndpoint::~PublisherEndpoint() {
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
