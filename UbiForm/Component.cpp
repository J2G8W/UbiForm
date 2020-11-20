#include <unistd.h>

#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"

#include "Component.h"

// Note that outgoing means it dials an external URL
void Component::createPairConnectionOutgoing(const char *url) {
    int rv;
    if ((rv = nng_pair0_open(&socket)) != 0) {
        fatal("nng_pair0_open", rv);
    }
    if ((rv = nng_dial(socket, url, nullptr, 0)) != 0) {
        fatal("nng_dial", rv);
    }
    sendManifestOnSocket();
    receiveManifestOnSocket();
}

// Incoming means it will listen on an internal URL
void Component::createPairConnectionIncoming(const char *url) {
    int rv;
    if ((rv = nng_pair0_open(&socket)) != 0) {
        fatal("nng_pair0_open", rv);
    }

    if ((rv = nng_listen(socket, url, nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    receiveManifestOnSocket();
    sendManifestOnSocket();
}

// We send out manifest out on the socket that the component has
void Component::sendManifestOnSocket() {
    // Assert that socket is open to something??
    int rv;

    std::string manifestTextObject = componentManifest->stringify();
    const char *manifestText = manifestTextObject.c_str();

    if ((rv = nng_send(socket, (void *) manifestText, strlen(manifestText), 0)) != 0) {
        fatal("nng_send (manifest)", rv);
    }
}

// Receive the manifest on the component's socket
void Component::receiveManifestOnSocket() {
    // Assert that socket is open to something??
    int rv;
    char *buffer = nullptr;
    size_t sz;

    // Is effectively a blocking receive which waits until a valid manifest comes in until it returns
    while (true) {
        if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
            try {
                socketManifest = std::make_unique<ComponentManifest>(buffer);
                nng_free(buffer, sz);
                break;
            } catch (std::logic_error &e) {
                std::cerr << "Manifest received not valid: " << e.what();
            }
        } else {
            std::cerr << "NNG error receiving message" << std::endl;
        }
    }
}

// Send the SocketMessage object on our socket after checking that our message is valid against our manifest
void Component::sendMessage(SocketMessage &s) {
    int rv;
    std::string messageTextObject = s.stringify();
    // Effectively treat this as cast, as the pointer is still to stack memory
    const char *buffer = messageTextObject.c_str();

    try {
        componentManifest->validate(s);
        if ((rv = nng_send(socket, (void *) buffer, strlen(buffer) + 1, 0)) != 0) {
            fatal("nng_send (msg)", rv);
        }
    } catch (std::logic_error &e){
        std::cerr << "Message couldn't send as: " << e.what();
    }
}

// Receive a message, validate it against the socketManifest and return a pointer to the object.
// Use smart pointers to avoid complex memory management
std::unique_ptr<SocketMessage> Component::receiveMessage() {
    int rv;
    char *buffer = nullptr;

    size_t sz;

    // Is effectively a blocking receive which waits until the correct information comes in until it returns
    while (true) {
        if ((rv = nng_recv(socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
            try {
                std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
                socketManifest->validate(*receivedMessage);
                nng_free(buffer, sz);
                return receivedMessage;
            } catch (std::logic_error &e) {
                std::cerr << "Message received not valid: " << e.what();
            }
        } else {
            std::cerr << "NNG error receiving message" << std::endl;
        }
    }
}

// Destructor waits a short time before closing socket such that any unsent messages are released
Component::~Component() {
    int rv;
    // Make sure that the messages are flushed
    sleep(1);
    if ((rv = nng_close(socket)) == NNG_ECLOSED) {
        std::cerr << "This socket had already been closed" << std::endl;
    }
}