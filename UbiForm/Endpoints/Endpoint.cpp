#include "../../include/UbiForm/Endpoints/Endpoint.h"

void Endpoint::startConnectionThread() {
    if(startupFunction != nullptr) {
        connectionThread = std::thread(startupFunction, this, extraData);
        connectionThreadNeedsClosing = true;
    }
}

void Endpoint::endConnectionThread() {
    if(connectionThreadNeedsClosing) {
        connectionThread.join();
        connectionThreadNeedsClosing = false;
    }
}
