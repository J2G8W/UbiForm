#ifndef UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H


#include <nng/nng.h>
#include <string>
#include <thread>
#include <map>

#include "ComponentRepresentation.h"
#include "ResourceDiscoveryStore.h"
#include "../Endpoints/ReplyEndpoint.h"

/**
 * A class to represent our ResourceDiscoveryHubs, these are used to store information about components on the network
 * and serve up the data to ResourceDiscoveryConnEndpoints
 */
class ResourceDiscoveryHubEndpoint {
private:
    std::thread rdThread;

    ReplyEndpoint replyEndpoint;
    ResourceDiscoveryStore rdStore;
    int backgroundPort;

    static void rdBackground(ResourceDiscoveryHubEndpoint *);

public:
    explicit ResourceDiscoveryHubEndpoint(SystemSchemas &ss) : rdStore(ss),
        replyEndpoint(
                ss.getSystemSchema(SystemSchemaName::generalRDRequest).getInternalSchema(),
                ss.getSystemSchema(SystemSchemaName::generalRDResponse).getInternalSchema(),
                "ResourceDiscoveryHub",
                "ResourceDiscoveryHub") {}

    /**
     * Start the ResourceDiscoveryHub. In starting it we start a new thread
     * @param baseAddress - The address without the ":port" bit
     * @param port - The port we want to listen on
     * @throws NngError when we fail to listen on the given port (and no backgroundProcess is started)
     */
    void startResourceDiscover(const std::string &baseAddress, int port);

    int getBackgroundPort() { return backgroundPort; }

    ~ResourceDiscoveryHubEndpoint();

};


#endif //UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
