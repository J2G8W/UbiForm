#include "Component.h"

#include "general_functions.h"

// Note that outgoing means it dials an external URL
void Component::createPairConnectionOutgoing(const char *url) {
    int rv;
    if((rv = nng_pair0_open(&socket)) != 0 ){
        fatal("nng_pair0_open", rv);
    }
    if ((rv = nng_dial(socket,url, nullptr, 0)) != 0){
        fatal("nng_dial",rv);
    }
}

// Incoming means it will listen on an internal URL
void Component::createPairConnectionIncoming(const char *url) {
    int rv;
    if((rv = nng_pair0_open(&socket)) != 0 ){
        fatal("nng_pair0_open", rv);
    }
    if ((rv = nng_listen(socket,url, nullptr, 0)) != 0){
        fatal("nng_dial",rv);
    }
}

void Component::sendManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;
    const char *manifestText = manifest.stringify();
    if ((rv = nng_send(socket, (void*) manifestText, strlen(manifestText) + 1,0)) != 0){
        fatal("nng_send", rv);
    }
}

void Component::receiveManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;
    char *buffer = nullptr;
    size_t  sz;
    if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
        std::cout << buffer << "\n";
        nng_free(buffer, sz);
    }else{
        fatal("nng_receive",rv);
    }
}