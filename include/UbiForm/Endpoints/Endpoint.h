#ifndef UBIFORM_ENDPOINT_H
#define UBIFORM_ENDPOINT_H

#include <thread>
#include "../Utilities/SystemEnums.h"


class Endpoint {
protected:
    EndpointState endpointState = EndpointState::Closed;

    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;


    std::thread connectionThread;
    bool connectionThreadNeedsClosing = false;
    endpointStartupFunction startupFunction;
    void* extraData;

public:
    Endpoint(const std::string &endpointIdentifier,
             SocketType socketType, const std::string &endpointType,
             endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
    endpointIdentifier(endpointIdentifier), socketType(socketType), endpointType(endpointType),
    startupFunction(startupFunction), extraData(extraData) {}


    EndpointState getEndpointState(){return endpointState;}
    /**
     * @return The identifier of the socket in the component
     */
    std::string &getEndpointId() { return endpointIdentifier; }

    /**
     * @return The endpointType (which refers to our componentManifest)
     */
    std::string &getEndpointType() { return endpointType; }

    SocketType getEndpointSocketType(){return  socketType;}

    void startConnectionThread();

    void endConnectionThread();

    ~Endpoint(){
        endConnectionThread();
    }
};


#endif //UBIFORM_ENDPOINT_H
