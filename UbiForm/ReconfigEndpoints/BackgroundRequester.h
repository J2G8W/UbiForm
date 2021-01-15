#ifndef UBIFORM_BACKGROUNDREQUESTER_H
#define UBIFORM_BACKGROUNDREQUESTER_H

#include "../SystemSchemas/SystemSchemas.h"
#include "../Endpoints/RequestEndpoint.h"
#include "../ComponentManifest.h"

class Component;
class BackgroundRequester {
    Component * component;
    SystemSchemas& systemSchemas;
    RequestEndpoint requestEndpoint;

    std::unique_ptr<SocketMessage> sendRequest(const std::string &url, SocketMessage & request);

public:
    BackgroundRequester(Component* c , SystemSchemas& ss):component(c), systemSchemas(ss),
    // Purposely make the request endpoint have an empty schema
                                                          requestEndpoint(ss.getSystemSchema(
                                                                  SystemSchemaName::generalEndpointResponse).getInternalSchema(),
                                                                          ss.getSystemSchema(
                                                                                  SystemSchemaName::generalEndpointRequest).getInternalSchema(),
                                                                          "BackgroundRequester",
                                                                          "BackgroundRequester") {}

    void requestAndCreateConnection(const std::string &baseAddress, int port,
                                    const std::string &localEndpointType,
                                    const std::string &remoteEndpointType);

    void requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl);

    void tellToRequestAndCreateConnection(const std::string &requesterAddress,
                                          const std::string &requesterEndpointType,
                                          const std::string &remoteEndpointType,
                                          const std::string &remoteAddress, int newPort);

    void requestChangeEndpoint(const std::string &componentAddress, SocketType socketType,
                               const std::string &endpointType, EndpointSchema *receiverSchema,
                               EndpointSchema *sendSchema);

    int requestCreateRDH(const std::string& componentUrl);
    void requestToCreateAndDial(const std::string& componentUrl, const std::string &socketType,
                                const std::string &endpointType, const std::string &remoteUrl);

    void requestUpdateComponentManifest(const std::string &componentUrl, ComponentManifest& newManifest);

    std::vector<std::string> requestLocationsOfRDH(const std::string& componentUrl);
    void requestCloseSocketOfType(const std::string& componentUrl, const std::string& endpointType);

    void requestCloseRDH(const std::string& componentUrl);

};


#endif //UBIFORM_BACKGROUNDREQUESTER_H
