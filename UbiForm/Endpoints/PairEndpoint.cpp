#include <string>

#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>


#include "../Utilities/UtilityFunctions.h"
#include "../../include/UbiForm/Endpoints/PairEndpoint.h"
#include "../Utilities/base64.h"


// Destructor waits a short time before closing socket such that any unsent messages are released
PairEndpoint::~PairEndpoint() {
    // We have to check if we ever initialised the receiverSocket before trying to close it
    if (senderSocket != nullptr) {
        if (!(endpointState == EndpointState::Closed ||
             endpointState == EndpointState::Invalid)) {
            nng_msleep(300);
            if (nng_close(*senderSocket) == NNG_ECLOSED) {
                std::cerr << "This socket had already been closed" << std::endl;
            } else {
                std::cout << "Pair socket " << endpointIdentifier << " closed" << std::endl;
            }
            endpointState = EndpointState::Invalid;
        }
    }
    if(receiverThreadNeedsClosing){
        receiverStreamingThread.join();
    }
    if(senderThreadNeedsClosing) {
        senderStreamingThread.join();
    }
    // Note that we only delete once as the senderSocket points to the same place as the receiverSocket
    delete senderSocket;
}

void PairEndpoint::closeEndpoint() {
    DataSenderEndpoint::closeEndpoint();

    if(receiverThreadNeedsClosing){
        receiverStreamingThread.join();
        receiverThreadNeedsClosing = false;
    }
    if(senderThreadNeedsClosing) {
        senderStreamingThread.join();
        senderThreadNeedsClosing = false;
    }
}

void PairEndpoint::openEndpoint() {
    if (endpointState == EndpointState::Closed) {
        int rv;
        if ((rv = nng_pair0_open(senderSocket)) != 0) {
            throw NngError(rv, "Making pair connection");
        } else {
            endpointState = EndpointState::Open;
        }
        // Use the same socket for sending and receiving
        receiverSocket = senderSocket;
    } else {
        throw SocketOpenError("Can't open endpoint", socketType,
                              endpointIdentifier);
    }
}

void PairEndpoint::listenForConnection(const char *base, int port) {
    DataSenderEndpoint::listenForConnection(base, port);
}

int PairEndpoint::listenForConnectionWithRV(const char *base, int port) {
    int rv = DataSenderEndpoint::listenForConnectionWithRV(base, port);
    return rv;
}

void PairEndpoint::dialConnection(const std::string &url) {
    DataReceiverEndpoint::dialConnection(url);
}


std::unique_ptr<SocketMessage>
PairEndpoint::receiveStream(std::ostream &outputStream, endOfStreamCallback endCallback, void *userData) {
    std::unique_ptr<SocketMessage> initialMsg;

    // Throws things
    initialMsg = receiveMessage();
    SocketMessage sm;
    sm.addMember("ready",true);
    rawSendMessage(sm);

    receiverThreadNeedsClosing = true;
    receiverThreadEnded = false;
    receiverStreamingThread = std::thread(streamReceiveData, this, &outputStream, endCallback, userData);


    return initialMsg;
}

void
PairEndpoint::streamReceiveData(PairEndpoint *endpoint, std::ostream *stream, endOfStreamCallback endCallback,
                                void *userData) {
    while(true) {
        std::unique_ptr<SocketMessage> message;
        try {
            message = endpoint->rawReceiveMessage();
        }catch(std::logic_error &e){
            std::cerr << e.what() << std::endl;
            break;
        }
        if(message->hasMember("end") && message->getBoolean("end")){
            break;
        }
        std::string r  = message->getString("bytes");
        base64_decode_to_stream(r,*stream);
    }
    if(endCallback != nullptr){
        endCallback(endpoint, userData);
    }
    endpoint->receiverThreadEnded = true;
}

void PairEndpoint::sendStream(std::istream &input, std::streamsize blockSize, bool holdWhenStreamEmpty,
                              SocketMessage &initialMessage,
                              endOfStreamCallback endCallback, void *userData) {
    if(blockSize % 3 != 0){throw std::logic_error("Block size must be a multiple of 3");}

    sendMessage(initialMessage);
    auto reply = rawReceiveMessage();
    if(!(reply->hasMember("ready") && reply->getBoolean("ready"))){
        throw std::logic_error("Unable to start sending stream");
    }

    senderThreadEnded = false;
    senderThreadNeedsClosing = true;
    this->senderStreamingThread = std::thread(streamSendData, this, &input, blockSize,holdWhenStreamEmpty, endCallback, userData);
}

void PairEndpoint::streamSendData(PairEndpoint *endpoint, std::istream *stream, std::streamsize blockSize,
                                  bool holdWhenStreamEmpty,
                                  endOfStreamCallback endCallback, void *userData) {
    char bytesToEncode[blockSize];
    int numBytes;
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
    if(endCallback != nullptr) {
        endCallback(endpoint, userData);
    }
    endpoint->senderThreadEnded = true;
}
