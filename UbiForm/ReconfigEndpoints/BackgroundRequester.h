#ifndef UBIFORM_BACKGROUNDREQUESTER_H
#define UBIFORM_BACKGROUNDREQUESTER_H

#include "../SystemSchemas/SystemSchemas.h"
#include "../Endpoints/RequestEndpoint.h"

class Component;
class BackgroundRequester {
    Component * component;
    SystemSchemas& systemSchemas;
    RequestEndpoint requestEndpoint;
public:
    BackgroundRequester(Component* c , SystemSchemas& ss):component(c), systemSchemas(ss),
    // Purposely make the request endpoint have an empty schema
        requestEndpoint(std::make_shared<EndpointSchema>(),std::make_shared<EndpointSchema>(), "BackgroundRequester"){}

    void requestAndCreateConnection(const std::string &connectionComponentAddress,
                                    const std::string &localEndpointType,
                                    const std::string &remoteEndpointType);

    void requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl);

    void tellToRequestAndCreateConnection(const std::string &requesterAddress,
                                          const std::string &requesterEndpointType,
                                          const std::string &remoteEndpointType,
                                          const std::string &remoteAddress);

    void requestChangeEndpoint(const std::string &componentAddress, SocketType socketType,
                               const std::string &endpointType, EndpointSchema *receiverSchema,
                               EndpointSchema *sendSchema);

    std::string requestCreateRDH(const std::string& componentUrl);
    void requestToCreateAndDial(const std::string& componentUrl, const std::string &socketType,
                                const std::string &endpointType, const std::string &remoteUrl);

    void requestUpdateComponentManifest(const std::string& componentUrl);
    std::vector<std::string> requestLocationsOfRDH(const std::string& componentUrl);
    void requestCloseSocketOfType(const std::string& componentUrl, const std::string endpointType);


};


#endif //UBIFORM_BACKGROUNDREQUESTER_H
