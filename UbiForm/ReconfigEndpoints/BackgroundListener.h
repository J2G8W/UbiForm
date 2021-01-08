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
#define ADD_ENDPOINT_SCHEMA "add_endpoint_schema"

class Component;
class BackgroundListener {
private:
    std::thread backgroundThread;
    std::string backgroundListenAddress;
    Component * component;
    SystemSchemas & systemSchemas;
    ReplyEndpoint replyEndpoint;

public:
    BackgroundListener(Component * c, SystemSchemas & ss) : component(c), systemSchemas(ss),
    replyEndpoint(std::make_shared<EndpointSchema>(), std::make_shared<EndpointSchema>(), "BackgroundListenerReply"){
    }

    void startBackgroundListen(const std::string& listenAddress);

    static void backgroundListen(BackgroundListener *backgroundListener);

    std::string getBackgroundListenAddress(){return backgroundListenAddress;}

    ~BackgroundListener();

    std::unique_ptr<SocketMessage> handleConnectionRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleAddRDH(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleTellCreateConnectionRequest(SocketMessage &request);

    std::unique_ptr<SocketMessage> handleAddEndpointRequest(SocketMessage &request);
};


#endif //UBIFORM_BACKGROUNDLISTENER_H
