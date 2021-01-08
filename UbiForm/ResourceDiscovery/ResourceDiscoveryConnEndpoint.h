#ifndef UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H

#include <vector>
#include <string>
#include "ComponentRepresentation.h"

class Component ;
class ResourceDiscoveryConnEndpoint {
private:
    // Map from url to our id on that rdh
    std::map<std::string, std::string> resourceDiscoveryHubs;
    Component * component;

    SystemSchemas & systemSchemas;

    std::unique_ptr<SocketMessage> sendRequest(const std::string &url, SocketMessage * request);

    ResourceDiscoveryConnEndpoint(ResourceDiscoveryConnEndpoint &) = delete;
    ResourceDiscoveryConnEndpoint& operator=(ResourceDiscoveryConnEndpoint &) = delete;

public:
    ResourceDiscoveryConnEndpoint(Component *component, SystemSchemas & ss) : component(component), systemSchemas(ss) {}


    SocketMessage *generateRegisterRequest();
    void registerWithHub(const std::string& url);

    std::vector<std::string> getComponentIdsFromHub(const std::string& url);

    std::unique_ptr<ComponentRepresentation> getComponentById(const std::string& url, const std::string& id);

    SocketMessage *generateFindBySchemaRequest(const std::string& endpointType);
    std::vector<std::unique_ptr<SocketMessage>> getComponentsBySchema(const std::string& endpointType);

    void createEndpointBySchema(const std::string& endpointType);

    std::string getId(const std::string& RdhUrl){
        return resourceDiscoveryHubs.at(RdhUrl);
    }
    std::vector<std::string> getResourceDiscoveryHubs(){
        std::vector<std::string> rdhs;
        for(auto url : resourceDiscoveryHubs){
            rdhs.push_back(url.first);
        }
        return rdhs;
    }

    void updateManifestWithHubs();

};


#endif //UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
