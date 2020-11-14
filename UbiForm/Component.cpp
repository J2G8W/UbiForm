#include <unistd.h>
#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"

#include "Component.h"

#include "general_functions.h"

// Note that outgoing means it dials an external URL
void Component::createPairConnectionOutgoing(const char *url) {
    int rv;
    if((rv = nng_pair0_open(&socket)) != 0 ){
        fatal("nng_pair1_open", rv);
    }
    if ((rv = nng_dial(socket,url, NULL, 0)) != 0){
        fatal("nng_dial",rv);
    }
}

// Incoming means it will listen on an internal URL
void Component::createPairConnectionIncoming(const char *url) {
    int rv;
    if((rv = nng_pair0_open(&socket)) != 0 ){
        fatal("nng_pair1_open", rv);
    }

    if ((rv = nng_listen(socket,url, NULL, 0)) != 0){
        fatal("nng_listen",rv);
    }
}

// We send out manifest out on the socket that the component has
void Component::sendManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;

    char * manifestText = manifest->stringify();
    std::cout << "SENDING: " << manifestText << "\n";

    //const char *manifestText = "HELLO WORLD";
    if ((rv = nng_send(socket, (void*) manifestText, strlen(manifestText),0)) != 0){
        fatal("nng_send", rv);
    }
}

// Receive the manifest on the component's socket
void Component::receiveManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;
    char *buffer = NULL;
    size_t  sz;

    if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
        std::cout << buffer << "\n";
        nng_free(buffer, sz);
    }else{
        fatal("nng_receive",rv);
    }
}

Component::~Component() {
    int rv;
    // Make sure that the messages are flushed
    sleep(1);
    if ((rv = nng_close(socket)) == NNG_ECLOSED) {
        std::cerr << "This socket had already been closed" << "\n";
    }
}