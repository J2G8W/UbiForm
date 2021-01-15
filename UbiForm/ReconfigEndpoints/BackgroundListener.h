#ifndef UBIFORM_BACKGROUNDLISTENER_H
#define UBIFORM_BACKGROUNDLISTENER_H

#include <thread>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include "../SystemSchemas/SystemSchemas.h"
#include "../Endpoints/ReplyEndpoint.h"

class Component;
class BackgroundListener {
private:
    std::thread backgroundThread;
    int backgroundPort = -1;
    Component * component;
    SystemSchemas & systemSchemas;
    ReplyEndpoint replyEndpoint;

public:
    BackgroundListener(Component * c, SystemSchemas & ss) : component(c), systemSchemas(ss),
                                                            replyEndpoint(
                                                                    ss.getSystemSchema(
                                                                            SystemSchemaName::generalEndpointRequest).getInternalSchema(),
                                                                    ss.getSystemSchema(
                                                                            SystemSchemaName::generalEndpointResponse).getInternalSchema(),
                                                                    "BackgroundListenerReply",
                                                                    "BackgroundListenerReply") {
    }

    void startBackgroundListen(const std::string &baseAddress, int port);

    static void backgroundListen(BackgroundListener *backgroundListener);

    int getBackgroundPort(){return backgroundPort;}

    ~BackgroundListener();

    std::unique_ptr<SocketMessage> handleConnectionRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleAddRDH(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleTellCreateConnectionRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleChangeEndpointRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCreateRDHRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleChangeManifestRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleRDHLocationsRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCloseSocketsRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleCloseRDH(SocketMessage &request);
};


#endif //UBIFORM_BACKGROUNDLISTENER_H
