#ifndef UBIFORM_BACKGROUNDREQUESTER_H
#define UBIFORM_BACKGROUNDREQUESTER_H

#include "../../../UbiForm/SystemSchemas/SystemSchemas.h"
#include "../Endpoints/RequestEndpoint.h"
#include "../ComponentManifest.h"

class Component;

/**
 * This class is used to send requests to a background listener on another component. Note that we will directly throw
 * errors from this if the request does not work. Look at sendRequest() to see what errors can be thrown.
 */
class BackgroundRequester {
    Component *component;
    SystemSchemas &systemSchemas;
    RequestEndpoint requestEndpoint;

    /**
     * Used to send our request after building it in the other methods
     * @param url - The url of the listener we want to dial
     * @param request - The message we want to send (built by other methods)
     * @return The reply of the listener to our request (with error set to false)
     * @throws NngError when we have problems dialing/sending/receiving our request
     * @throws RemoteError when the remote address has an error in our request
     * @throws ValidationError when our request/response doesn't have the right setup
     */
    std::unique_ptr<EndpointMessage> sendRequest(const std::string &url, EndpointMessage &request);

    // We purposely delete copy and assignment operators such that there isn't stray references to this
    BackgroundRequester(BackgroundRequester &) = delete;
    BackgroundRequester &operator=(BackgroundRequester &) = delete;

public:
    BackgroundRequester(Component *c, SystemSchemas &ss) : component(c), systemSchemas(ss),
    requestEndpoint(ss.getSystemSchema(SystemSchemaName::generalEndpointResponse).getInternalSchema(),
                    ss.getSystemSchema(SystemSchemaName::generalEndpointRequest).getInternalSchema(),
                    "BackgroundRequester","BackgroundRequester") {}

    void requestRemoteListenThenDial(const std::string &locationOfRemote, int remotePort,
                                     const std::string &localEndpointType,
                                     const std::string &remoteEndpointType);

    int requestToCreateAndListen(const std::string &componentAddress, const std::string &endpointType);

    void localListenThenRequestRemoteDial(const std::string &componentAddress, const std::string &localEndpointType,
                                          const std::string &remoteEndpointType);

    void requestToCreateAndDial(const std::string &componentUrl, const std::string &endpointType,
                                const std::vector<std::string> &remoteUrls);

    void request3rdPartyRemoteListenThenDial(const std::string &requesterAddress,
                                             const std::string &requesterEndpointType,
                                             const std::string &remoteEndpointType,
                                             const std::string &remoteAddress, int remotePort);

    void request3rdPartyListenThenRemoteDial(const std::string &listenAddress, const std::string &listenEndpointType,
                                             const std::string &dialEndpointType, const std::string &dialerAddress);

    int requestCreateRDH(const std::string &componentUrl);

    void requestCloseRDH(const std::string &componentUrl);

    void requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl);

    void requestRemoveRDH(const std::string& componentUrl, const std::string& rdhUrl);

    std::vector<std::string> requestLocationsOfRDH(const std::string &componentUrl);


    void requestUpdateComponentManifest(const std::string &componentUrl, ComponentManifest &newManifest);

    void requestChangeEndpoint(const std::string &componentAddress, ConnectionParadigm connectionParadigm,
                               const std::string &endpointType, EndpointSchema *receiverSchema,
                               EndpointSchema *sendSchema);

    void requestCloseEndpointsOfType(const std::string &componentUrl, const std::string &endpointType);

    std::vector<std::unique_ptr<EndpointMessage>> requestEndpointInfo(const std::string &componentUrl);

    void requestCloseEndpointOfId(const std::string &componentUrl, const std::string &endpointId);

    std::unique_ptr<ComponentManifest> requestComponentManifest(const std::string &componentUrl);
};


#endif //UBIFORM_BACKGROUNDREQUESTER_H
