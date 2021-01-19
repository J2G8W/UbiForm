#ifndef UBIFORM_BACKGROUNDLISTENER_H
#define UBIFORM_BACKGROUNDLISTENER_H

#include <thread>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include "../SystemSchemas/SystemSchemas.h"
#include "../Endpoints/ReplyEndpoint.h"

class Component;

/**
 * This class is used to listen on the background of a component. This uses the UbiForm Reply endpoint to do most of the heavy
 * lifting, and once started should sit in the background actioning requests from other components to do stuff
 */
class BackgroundListener {
private:
    std::thread backgroundThread;
    int backgroundPort = -1;
    Component *component;
    SystemSchemas &systemSchemas;
    ReplyEndpoint replyEndpoint;

    ///@{
    std::unique_ptr<SocketMessage> handleCreateAndListen(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleAddRDH(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleRemoveRDH(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleTellCreateConnectionRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleChangeEndpointRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCreateRDHRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleChangeManifestRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleRDHLocationsRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCloseSocketsRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCloseEndpointByIdRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCloseRDH(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCreateAndDial(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleEndpointInfoRequest(SocketMessage &request);
    ///@}

public:
    BackgroundListener(Component *c, SystemSchemas &ss) : component(c), systemSchemas(ss),
                                                          replyEndpoint(ss.getSystemSchema(
                                                                  SystemSchemaName::generalEndpointRequest).getInternalSchema(),
                                                                        ss.getSystemSchema(
                                                                                SystemSchemaName::generalEndpointResponse).getInternalSchema(),
                                                                        "BackgroundListenerReply",
                                                                        "BackgroundListenerReply") {
    }

    /**
     * When this is called we start our listener on the given port. It starts a background listener to respond to request
     * @param baseAddress - the baseAddress to listen on (tcp:// * is often given to mean listen on all address)
     * @param port
     * @throws NngError when we are unable to listen on the given port.
     */
    void startBackgroundListen(const std::string &baseAddress, int port);

    /**
     * This is the actual listener which runs in the background on our thread. To do most of its processing it passes
     * request to its handler. When it encounters NngErrors it simply ends itself as this is unrectifyable, but for all
     * other errors it will return a SocketMessage detailing the error (without actually throwing any exceptions itself)
     * @param backgroundListener - the listener it uses
     */
    static void backgroundListen(BackgroundListener *backgroundListener);

    int getBackgroundPort() { return backgroundPort; }

    ~BackgroundListener();

};


#endif //UBIFORM_BACKGROUNDLISTENER_H
