#ifndef UBIFORM_ENDPOINT_H
#define UBIFORM_ENDPOINT_H

#include <thread>
#include "../Utilities/SystemEnums.h"


class Endpoint {
protected:
    EndpointState endpointState = EndpointState::Closed;

    std::string endpointIdentifier;
    std::string endpointType;
    ConnectionParadigm connectionParadigm;


    std::thread connectionThread;
    bool connectionThreadNeedsClosing = false;
    endpointStartupFunction startupFunction;
    void* extraData;

public:
    Endpoint(const std::string &endpointIdentifier,
             ConnectionParadigm cp, const std::string &endpointType,
             endpointStartupFunction startupFunction = nullptr, void* extraData = nullptr) :
            endpointIdentifier(endpointIdentifier), connectionParadigm(cp), endpointType(endpointType),
            startupFunction(startupFunction), extraData(extraData) {}


    EndpointState getEndpointState(){return endpointState;}
    /**
     * @return The identifier of the endpoint in the component
     */
    std::string &getEndpointId() { return endpointIdentifier; }

    /**
     * @return The endpointType (which refers to our componentManifest)
     */
    std::string &getEndpointType() { return endpointType; }

    ConnectionParadigm getConnectionParadigm(){return  connectionParadigm;}

    void startConnectionThread();

    void endConnectionThread();

    void invalidateEndpoint(){ endpointState = EndpointState::Invalid;}

    virtual void closeEndpoint() = 0;

    virtual ~Endpoint(){
        endConnectionThread();
    }
};


#endif //UBIFORM_ENDPOINT_H
