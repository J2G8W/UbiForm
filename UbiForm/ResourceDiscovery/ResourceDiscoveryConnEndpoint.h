#ifndef UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H

#include <vector>
#include <string>
#include "ComponentRepresentation.h"

class Component ;
/**
 * A class which represents how we contact ResourceDiscoveryHubs. It provides the methods which you'd expect would be useful.
 * It currently makes a new NNG socket every time we want a request, may shift this to use UbiForm wrapper
 */
class ResourceDiscoveryConnEndpoint {
private:
    // Map from url to our id on that rdh
    std::map<std::string, std::string> resourceDiscoveryHubs;
    Component * component;

    SystemSchemas & systemSchemas;

    std::unique_ptr<SocketMessage> sendRequest(const std::string &url, SocketMessage * request);

    // We purposely delete copy and assignment operators such that there isn't stray references to this
    ResourceDiscoveryConnEndpoint(ResourceDiscoveryConnEndpoint &) = delete;
    ResourceDiscoveryConnEndpoint& operator=(ResourceDiscoveryConnEndpoint &) = delete;

public:
    ResourceDiscoveryConnEndpoint(Component *component, SystemSchemas & ss) : component(component), systemSchemas(ss) {}


    SocketMessage *generateRegisterRequest();
    /**
     * Send a request to a ResourceDiscoveryHub at url and if it comes back successfully we register it as a Hub we can talk to.
     * Note that it doesn't throw exceptions, just outputs to cerr
     * @param url - The COMPLETE url of the RDH
     */
    void registerWithHub(const std::string& url);

    /**
     * Request ALL the IDs that a hub has.
     * @param url - The COMPLETE url of the RDH
     * @return Vector of IDs that hub has or an empty vector on error in contact
     */
    std::vector<std::string> getComponentIdsFromHub(const std::string& url);

    /**
     * Request the representation of a single component from an RDH
     * @param url - The COMPLETE url of the RDH
     * @param id - The id of the component we want
     * @return std::unique_ptr to a ComponentRepresentation from the RDH
     * @throws std::logic_error when the RDH doesn't reply properly or has not component
     */
    std::unique_ptr<ComponentRepresentation> getComponentById(const std::string& url, const std::string& id);

    SocketMessage *generateFindBySchemaRequest(const std::string& endpointType,
                                               std::map<std::string, std::string> &otherValues);
    /**
     * Sends requests to all the RDHs we know about, for the components which match the endpointType we request.
     * Assumes we only want DataReceiverEndpoints back
     * @param endpointType - Reference to the endpointType in our componentManifest
     * @return Vector of SocketMessages (which handle memory themselves) which follow Schema "SystemsSchemas/resource_discovery_by_schema_response"
     */
    std::vector<std::unique_ptr<SocketMessage>>
    getComponentsBySchema(const std::string &endpointType, std::map<std::string, std::string> &otherValues);

    /**
     * Uses the getComponentsBySchema function to actually create connections to ALL of the available endpoints from our RDHs.
     * Should not throw any errors, we catch any creation errors. We do record in std::cerr when we can't connect to
     * any of the URLs for a component
     * @param endpointType - Reference to the endpointType in our ComponentManifest
     */
    void createEndpointBySchema(const std::string& endpointType);

    /**
     * Get our ID on the RDH represented
     * @param RdhUrl - RDH we want our ID for
     * @return ID
     * @throws std::out_of_range when we haven't registered with that RDH yet
     */
    std::string getId(const std::string& RdhUrl){
        return resourceDiscoveryHubs.at(RdhUrl);
    }

    /**
     * Get the COMPLETE urls of all the RDHs we are connected to
     * @return Vector of complete urls (including port numbers)
     */
    std::vector<std::string> getResourceDiscoveryHubs(){
        std::vector<std::string> rdhs;
        for(auto url : resourceDiscoveryHubs){
            rdhs.push_back(url.first);
        }
        return rdhs;
    }

    /**
     * Update our manifest with each RDH we are registered with
     */
    void updateManifestWithHubs();

    void searchForResourceDiscoveryHubs();

};


#endif //UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
