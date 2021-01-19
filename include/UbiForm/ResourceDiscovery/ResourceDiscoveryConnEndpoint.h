#ifndef UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H


#include <vector>
#include <string>
#include "../Component.h"
#include "ComponentRepresentation.h"

class Component ;
class ResourceDiscoveryConnEndpoint {
private:
    // Map from url to our id on that rdh
    std::map<std::string, std::string> resourceDiscoveryHubs;
    Component * component;

    SystemSchemas & systemSchemas;

    SocketMessage * sendRequest(const std::string&, SocketMessage * request);

public:
    ResourceDiscoveryConnEndpoint(Component *component, SystemSchemas & ss) : component(component), systemSchemas(ss) {}


    SocketMessage *generateRegisterRequest();
    void registerWithHub(const std::string& url);

    std::vector<std::string> getComponentIdsFromHub(const std::string& url);

    ComponentRepresentation * getComponentById(const std::string& url, const std::string& id);

    SocketMessage *generateFindBySchemaRequest(const std::string& endpointType);
    std::vector<SocketMessage *> getComponentsBySchema(const std::string& endpointType);

    void createEndpointBySchema(const std::string& endpointType);

    std::string getId(const std::string& RdhUrl){
        return resourceDiscoveryHubs.at(RdhUrl);
    }
};


#endif //UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H