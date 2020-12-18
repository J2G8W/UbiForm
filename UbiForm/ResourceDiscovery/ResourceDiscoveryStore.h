
#ifndef UBIFORM_RESOURCEDISCOVERYSTORE_H
#define UBIFORM_RESOURCEDISCOVERYSTORE_H


#include <map>
#include <random>
#include "../SocketMessage.h"
#include "ComponentRepresentation.h"
#include "../SystemSchemas/SystemSchemas.h"

class ResourceDiscoveryStore{
private:

    SystemSchemas &systemSchemas;

    std::map<std::string, std::shared_ptr<ComponentRepresentation>> componentById;
    static std::minstd_rand0 generator;



public:
    static SocketMessage * generateRDResponse(SocketMessage *sm, ResourceDiscoveryStore & rds);

    ResourceDiscoveryStore(SystemSchemas & ss);
};


#endif //UBIFORM_RESOURCEDISCOVERYSTORE_H
