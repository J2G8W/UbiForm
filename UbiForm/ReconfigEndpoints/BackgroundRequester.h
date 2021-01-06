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
        requestEndpoint(std::make_shared<EndpointSchema>(),std::make_shared<EndpointSchema>()){}

    void requestAndCreateConnection(const std::string &localEndpointType, const std::string &connectionComponentAddress,
                                    const std::string &remoteEndpointType);

    void requestAddRDH(const std::string &rdhUrl, const std::string &componentUrl);

    void tellToRequestAndCreateConnection(const std::string &requesterEndpointType, const std::string &requesterAddress,
                                          const std::string &remoteEndpointType, const std::string &remoteAddress);
};


#endif //UBIFORM_BACKGROUNDREQUESTER_H
