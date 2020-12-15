#ifndef UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H


#include <nng/nng.h>
#include <string>
#include <thread>
#include <map>
#include "ComponentRepresentation.h"

class ResourceDiscoveryHubEndpoint {
private:
    nng_socket rdSocket;
    std::thread rdThread;

    std::map<std::string, ComponentRepresentation *> componentById;


    static void rdBackground(ResourceDiscoveryHubEndpoint *);

    static SocketMessage * generateRDResponse(SocketMessage * sm, ResourceDiscoveryHubEndpoint *rdhe);
public:
    explicit ResourceDiscoveryHubEndpoint(std::string initUrl);


};


#endif //UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
