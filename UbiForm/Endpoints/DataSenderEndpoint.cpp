#include "../../include/UbiForm/Endpoints/DataSenderEndpoint.h"
#include "../Utilities/base64.h"
#include <nng/supplemental/util/platform.h>

// Send the SocketMessage object on our socket after checking that our message is valid against our manifest
void DataSenderEndpoint::sendMessage(SocketMessage &s) {
    if (!(endpointState == EndpointState::Listening || endpointState == EndpointState::Dialed)) {
        throw SocketOpenError("Could not send message, socket is closed", socketType, endpointIdentifier);
    }
    int rv;
    std::string messageTextObject = s.stringify();
    // Effectively treat this as cast, as the pointer is still to stack memory
    const char *buffer = messageTextObject.c_str();


    senderSchema->validate(s);
    if ((rv = nng_send(*senderSocket, (void *) buffer, strlen(buffer) + 1, 0)) != 0) {
        throw NngError(rv, "nng_send");
    }
}

void DataSenderEndpoint::asyncSendMessage(SocketMessage &s) {
    if (!(endpointState == EndpointState::Listening || endpointState == EndpointState::Dialed )) {
        throw SocketOpenError("Could not async-send message, socket is closed", socketType, endpointIdentifier);
    }
    nng_aio_wait(nngAioPointer);
    std::string text = s.stringify();
    const char *textArray = text.c_str();

    nng_msg *msg;
    int rv;
    if ((rv = nng_msg_alloc(&msg, 0)) != 0) {
        throw NngError(rv, "Allocating async message space");
    }
    if ((rv = nng_msg_append(msg, (void *) textArray, text.size() + 1)) != 0) {
        throw NngError(rv, "Creating Async Message");
    }
    nng_aio_set_msg(nngAioPointer, msg);

    nng_send_aio(*senderSocket, nngAioPointer);
}

void DataSenderEndpoint::asyncCleanup(void *data) {
    auto *asyncInput = static_cast<DataSenderEndpoint *>(data);

    if (nng_aio_result(asyncInput->nngAioPointer) != 0) {
        // Failed message send, we do cleanup
        asyncInput->numSendFails++;
        nng_msg *msg = nng_aio_get_msg(asyncInput->nngAioPointer);
        nng_msg_free(msg);
    }
}

void DataSenderEndpoint::setSendTimeout(int ms_time) {
    if (!(endpointState == EndpointState::Closed || endpointState == EndpointState::Invalid)) {
        int rv = nng_socket_set_ms(*senderSocket, NNG_OPT_SENDTIMEO, ms_time);
        if (rv != 0) {
            throw NngError(rv, "Set send timeout");
        }
    } else {
        throw SocketOpenError("Can't set timeout if endpoint not open", socketType, endpointIdentifier);
    }
}

void DataSenderEndpoint::listenForConnection(const char *base, int port) {
    int rv = listenForConnectionWithRV(base, port);
    if (rv != 0) {
        throw NngError(rv, "Listening on " + std::string(base));
    }
}

int DataSenderEndpoint::listenForConnectionWithRV(const char *base, int port) {
    if (endpointState == EndpointState::Open) {
        int rv;
        std::string addr = std::string(base) + ":" + std::to_string(port);
        if ((rv = nng_listen(*senderSocket, addr.c_str(), nullptr, 0)) != 0) {
            return rv;
        }
        this->endpointState = EndpointState::Listening;
        this->listenPort = port;
        return rv;
    } else {
        throw SocketOpenError("Can't listen if endpoint not open", socketType, endpointIdentifier);
    }
}

void DataSenderEndpoint::closeEndpoint() {
    if (!(endpointState == EndpointState::Closed ||
        endpointState == EndpointState::Invalid)) {
        if (nng_close(*senderSocket) == NNG_ECLOSED) {
            std::cerr << "This socket had already been closed" << std::endl;
        } else {
            std::cout << convertFromSocketType(socketType)<< " socket: " << DataSenderEndpoint::endpointIdentifier << " closed" << std::endl;
        }
        endpointState = EndpointState::Closed;
    }
    if(senderThreadOpen){
        senderStreamingThread.join();
        senderThreadOpen = false;
    }
}

void DataSenderEndpoint::sendStream(std::istream &input, std::streamsize blockSize, bool holdWhenStreamEmpty) {
    if(blockSize % 3 != 0){throw std::logic_error("Block size must be a multiple of 3");}

    senderThreadOpen = true;
    this->senderStreamingThread = std::thread(DataSenderEndpoint::streamData, this, &input, blockSize, holdWhenStreamEmpty);
}

void DataSenderEndpoint::streamData(DataSenderEndpoint *endpoint, std::istream *stream, std::streamsize blockSize,
                                    bool holdWhenStreamEmpty) {
    char bytesToEncode[blockSize];
    int numBytes = 1;
    while(endpoint->endpointState == EndpointState::Dialed || endpoint->endpointState == EndpointState::Listening) {
        stream->read(bytesToEncode, blockSize);
        numBytes = stream->gcount();

        if(numBytes == 0){
            if(holdWhenStreamEmpty) {
                nng_msleep(1000);
                continue;
            }else{
                SocketMessage sm;
                sm.addMember("end",true);
                try{
                    endpoint->asyncSendMessage(sm);
                    break;
                }catch (std::logic_error&e){
                    break;
                }
            }
        }

        std::string encodedMsg = base64_encode(reinterpret_cast<const unsigned char *>(bytesToEncode), numBytes);

        SocketMessage sm;
        sm.addMember("bytes", encodedMsg);
        try {
            endpoint->asyncSendMessage(sm);
        }catch(std::logic_error &e){
            break;
        }
    }
}