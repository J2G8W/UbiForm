#ifndef UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H


#include <nng/nng.h>
#include <string>
#include <thread>
#include <map>

#include "ComponentRepresentation.h"
#include "ResourceDiscoveryStore.h"
#include "../Endpoints/ReplyEndpoint.h"

class ResourceDiscoveryHubEndpoint {
private:
    std::thread rdThread;

    ReplyEndpoint replyEndpoint;
    ResourceDiscoveryStore rdStore;

    static void rdBackground(ResourceDiscoveryHubEndpoint *);

public:
    explicit ResourceDiscoveryHubEndpoint(SystemSchemas &ss) : rdStore(ss),
     replyEndpoint(std::make_shared<EndpointSchema>(), std::make_shared<EndpointSchema>()) {}
    void startResourceDiscover(const std::string& urlInit);

    ~ResourceDiscoveryHubEndpoint();

};



#endif //UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
