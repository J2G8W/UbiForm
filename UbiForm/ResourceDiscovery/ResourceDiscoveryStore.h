#ifndef UBIFORM_RESOURCEDISCOVERYSTORE_H
#define UBIFORM_RESOURCEDISCOVERYSTORE_H


#include <map>
#include <random>
#include "../SocketMessage.h"
#include "ComponentRepresentation.h"
#include "../SystemSchemas/SystemSchemas.h"

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
     * @param sm - Input SocketMessage
     * @return The reply SocketMessage (should be memory handled pls)
     */

    std::unique_ptr<SocketMessage> generateRDResponse(SocketMessage *sm);

    explicit ResourceDiscoveryStore(SystemSchemas &ss);
};


#endif //UBIFORM_RESOURCEDISCOVERYSTORE_H
