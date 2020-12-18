#ifndef UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H


#include <nng/nng.h>
#include <string>
#include <thread>
#include <map>

#include "ComponentRepresentation.h"
#include "ResourceDiscoveryStore.h"

class ResourceDiscoveryHubEndpoint {
private:
    nng_socket rdSocket;
    std::thread rdThread;

    ResourceDiscoveryStore rdStore;

    static void rdBackground(ResourceDiscoveryHubEndpoint *);

public:
    ResourceDiscoveryHubEndpoint(SystemSchemas &ss) : rdStore(ss), rdSocket() {}
    void startResourceDiscover(std::string urlInit);

};



#endif //UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
