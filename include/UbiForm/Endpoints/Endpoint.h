#ifndef UBIFORM_ENDPOINT_H
#define UBIFORM_ENDPOINT_H

#include "../Utilities/SystemEnums.h"


class Endpoint {
protected:
    EndpointState endpointState = EndpointState::Closed;

    std::string endpointIdentifier;
    std::string endpointType;
    SocketType socketType;
public:
    Endpoint(const std::string &endpointIdentifier,
             SocketType socketType, const std::string &endpointType) :
    endpointIdentifier(endpointIdentifier), socketType(socketType), endpointType(endpointType) {}


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
};


#endif //UBIFORM_ENDPOINT_H
