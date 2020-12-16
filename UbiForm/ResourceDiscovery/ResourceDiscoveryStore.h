
#ifndef UBIFORM_RESOURCEDISCOVERYSTORE_H
#define UBIFORM_RESOURCEDISCOVERYSTORE_H


#include <map>
#include "../SocketMessage.h"
#include "ComponentRepresentation.h"

class ResourceDiscoveryStore{
private:
    std::map<std::string, std::shared_ptr<ComponentRepresentation>> componentById;
public:
    static SocketMessage * generateRDResponse(SocketMessage *sm, ResourceDiscoveryStore & rds);


};

#endif //UBIFORM_RESOURCEDISCOVERYSTORE_H
