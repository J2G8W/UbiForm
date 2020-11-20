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
    if ((rv = nng_dial(socket,url, nullptr, 0)) != 0){
        fatal("nng_dial",rv);
    }
    sendManifestOnSocket();
    receiveManifestOnSocket();
}

// Incoming means it will listen on an internal URL
void Component::createPairConnectionIncoming(const char *url) {
    int rv;
    if((rv = nng_pair0_open(&socket)) != 0 ){
        fatal("nng_pair1_open", rv);
    }

    if ((rv = nng_listen(socket,url, nullptr, 0)) != 0){
        fatal("nng_listen",rv);
    }
    receiveManifestOnSocket();
    sendManifestOnSocket();
}

// We send out manifest out on the socket that the component has
void Component::sendManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;

    char * manifestText = componentManifest->stringify();

    if ((rv = nng_send(socket, (void*) manifestText, strlen(manifestText),0)) != 0){
        fatal("nng_send (manifest)", rv);
    }
}

// Receive the manifest on the component's socket
void Component::receiveManifestOnSocket(){
    // Assert that socket is open to something??
    int rv;
    char *buffer = nullptr;
    size_t  sz;

    if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
        socketManifest = std::make_unique<ComponentManifest>(buffer);
        nng_free(buffer, sz);
    }else{
        fatal("nng_receive (manifest)",rv);
    }
}

void Component::sendMessage(SocketMessage& s) {
    int rv;
    char* buffer = s.stringify();

    componentManifest->validate(s);

    if ((rv = nng_send(socket, (void*) buffer, strlen(buffer) + 1 ,0)) != 0){
        fatal("nng_send (msg)", rv);
    }
}

std::unique_ptr<SocketMessage> Component::receiveMessage() {
    int rv;
    char *buffer = nullptr;

    size_t  sz;

    if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
        std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
        socketManifest->validate(*receivedMessage);
        nng_free(buffer, sz);
        return receivedMessage;
    }else{
        fatal("nng_receive (manifest)",rv);
        // TODO - change this error handling
        return nullptr;
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
Component::~Component() {
    int rv;
    // Make sure that the messages are flushed
    sleep(1);
    if ((rv = nng_close(socket)) == NNG_ECLOSED) {
        std::cerr << "This socket had already been closed" << "\n";
    }
}