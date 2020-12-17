#ifndef UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H


#include <vector>
#include <string>
#include "../Component.h"
#include "ComponentRepresentation.h"

class Component ;
class ResourceDiscoveryConnEndpoint {
private:
    std::vector<std::string> RDHUrls;
    Component * component;

    SocketMessage * sendRequest(std::string, SocketMessage * request);

public:
    explicit ResourceDiscoveryConnEndpoint(Component *component) : component(component) {}

    void addResourceDiscoveryHub (std::string url){
        RDHUrls.push_back(url);
    }

    SocketMessage *generateRegisterRequest();
    void registerWithHub(std::string url);

    std::vector<std::string> getComponentIdsFromHub(std::string url);

    ComponentRepresentation * getComponentById(std::string url, std::string id);

    SocketMessage *generateFindBySchemaRequest(std::string endpointType);
    std::vector<SocketMessage *> getComponentsBySchema(std::string endpointType);

};


#endif //UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
