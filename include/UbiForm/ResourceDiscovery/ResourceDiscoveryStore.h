#ifndef UBIFORM_RESOURCEDISCOVERYSTORE_H
#define UBIFORM_RESOURCEDISCOVERYSTORE_H


#include <map>
#include <random>
#include "../EndpointMessage.h"
#include "ComponentRepresentation.h"
#include "../../../UbiForm/SystemSchemas/SystemSchemas.h"

/**
 * Class used by ResourceDiscoveryHub to make it easier to handle logic. It basically just generates responses to the
 * requrests it is passed. It doesn't care which Component it is attached so should get the same logic wherever
 */
class ResourceDiscoveryStore {
private:

    SystemSchemas &systemSchemas;

    std::map<std::string, std::shared_ptr<ComponentRepresentation>> componentById;

    std::minstd_rand0 generator;

public:
    /**
     * Basically just makes the ResourceDiscoveryHub response to some socketMessage
     * @param sm - Input EndpointMessage
     * @return The reply EndpointMessage (should be memory handled pls)
     */

    std::unique_ptr<EndpointMessage> generateRDResponse(EndpointMessage *sm);

    std::vector<std::shared_ptr<ComponentRepresentation>> getConnections();

    explicit ResourceDiscoveryStore(SystemSchemas &ss);
};


#endif //UBIFORM_RESOURCEDISCOVERYSTORE_H
