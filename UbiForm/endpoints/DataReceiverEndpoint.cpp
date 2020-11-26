#include "DataReceiverEndpoint.h"
#include <nng/nng.h>


// Receive a message, validate it against the socketManifest and return a pointer to the object.
// Use smart pointers to avoid complex memory management
std::unique_ptr<SocketMessage> DataReceiverEndpoint::receiveMessage() {
    int rv;
    char *buffer = nullptr;

    size_t sz;

    // Is effectively a blocking receive which waits until the correct information comes in until it returns
    while (true) {
        try{
            if ((rv = nng_recv(*receiverSocket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
                std::unique_ptr<SocketMessage> receivedMessage = std::make_unique<SocketMessage>(buffer);
                receiverSchema->validate(*receivedMessage);
                nng_free(buffer, sz);
                return receivedMessage;
            }else{
                throw NNG_error(rv, "nng_recv");
            }
        } catch(std::logic_error &e) {
            std::cerr << "Failed message receive send as: " << e.what() << std::endl;
        }
    }
}

struct work{
    nng_aio *aio;
    void (*callback)(SocketMessage *);
    std::shared_ptr<EndpointSchema> schema;
};
void asyncCallback(void* data){
    work * w = static_cast<work *>(data);

    nng_msg * msg = nng_aio_get_msg(w->aio);
    char* text = static_cast<char *>(nng_msg_body(msg));
    SocketMessage *receivedMessage = new SocketMessage(text);
    w->schema->validate(*receivedMessage);
    nng_msg_free(msg);

    w->callback(receivedMessage);
}



void DataReceiverEndpoint::asyncReceiveMessage(void (*callb)(SocketMessage *)) {
    struct work *w = new work();
    w->callback = callb;
    w->schema = this->receiverSchema;
    nng_aio_alloc(&(w->aio), asyncCallback, w);

    nng_recv_aio(*receiverSocket, w->aio);

}
