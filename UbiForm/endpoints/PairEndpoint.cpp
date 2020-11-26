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
    }
    // Use the same socket for sending and receiving
    receiverSocket = senderSocket;
    if ((rv = nng_dial(*senderSocket, url, nullptr, 0)) != 0) {
        fatal("nng_dial", rv);
    }

}

// Incoming means it will listen on an internal URL
void PairEndpoint::listenForConnection(const char *url){
    int rv;
    if ((rv = nng_pair0_open(senderSocket)) != 0) {
        fatal("nng_pair0_open", rv);
    }
    // Use the same socket for sending and receiving
    receiverSocket = senderSocket;

    if ((rv = nng_listen(*senderSocket, url, nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }

}