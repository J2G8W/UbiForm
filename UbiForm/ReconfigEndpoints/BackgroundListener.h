#ifndef UBIFORM_BACKGROUNDLISTENER_H
#define UBIFORM_BACKGROUNDLISTENER_H

#include <thread>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include "../SystemSchemas/SystemSchemas.h"
#include "../Endpoints/ReplyEndpoint.h"

#define REQ_CONN "req_conn"
#define ADD_RDH "add_rdh"
#define TELL_REQ_CONN "tell_req_conn"
#define CHANGE_ENDPOINT_SCHEMA "add_endpoint_schema"
#define CREATE_RDH "create_rdh"
#define CHANGE_MANIFEST "change_manifest"
#define LOCATIONS_OF_RDH "locations_rdh"
#define CLOSE_SOCKETS "close_sockets"
#define CLOSE_RDH "close_rdh"

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
    replyEndpoint(ss.getSystemSchema(SystemSchemaName::generalEndpointResponse).getInternalSchema(),
                  ss.getSystemSchema(SystemSchemaName::generalEndpointRequest).getInternalSchema(),
                  "BackgroundListenerReply"){
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
