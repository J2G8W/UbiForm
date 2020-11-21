#include "DataReceiverEndpoint.h"

// Receive a message, validate it against the socketManifest and return a pointer to the object.
// Use smart pointers to avoid complex memory management
std::unique_ptr<SocketMessage> DataReceiverEndpoint::receiveMessage() {
    int rv;
    char *buffer = nullptr;

    size_t sz;

    // Is effectively a blocking receive which waits until the correct information comes in until it returns
    while (true) {
        if ((rv = nng_recv(*receiverSocket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
            try {
                std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
                receiverSchema.validate(*receivedMessage);
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