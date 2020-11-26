#include "SubscriberEndpoint.h"

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include <unistd.h>

void SubscriberEndpoint::dialConnection(const char *url){
    int rv;
    if ((rv = nng_sub0_open(receiverSocket)) != 0) {
        fatal("nng_sub0_open", rv);
    }
    if ((rv = nng_dial(*receiverSocket, url, nullptr, 0)) != 0) {
        fatal("nng_dial", rv);
    }

    // Default is that we subscribe to all topics
    if ((rv = nng_socket_set(*receiverSocket, NNG_OPT_SUB_SUBSCRIBE, nullptr, 0)) != 0) {
        fatal("nng_socket_set", rv);
    }

}

// Destructor waits a short time before closing socket such that any unsent messages are released
SubscriberEndpoint::~SubscriberEndpoint() {
    int rv;
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (receiverSocket != nullptr) {
        // Make sure that the messages are flushed
        sleep(1);
        // We only have one actual socket so only need to close 1.
        if ((rv = nng_close(*receiverSocket)) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        }

    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete receiverSocket;
}