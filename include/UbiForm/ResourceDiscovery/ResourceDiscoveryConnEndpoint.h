#ifndef UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H

#include <vector>
#include <string>
#include "ComponentRepresentation.h"
#include "../Endpoints/RequestEndpoint.h"

class Component;


class ResourceDiscoveryConnEndpoint {
private:
    // Map from url to our id on that rdh
    std::map<std::string, std::string> resourceDiscoveryHubs;
    std::map<std::string, std::unique_ptr<RequestEndpoint> > resourceDiscoveryEndpoints;
    Component *component;

    SystemSchemas &systemSchemas;

    static void handleAsyncReceive(EndpointMessage* sm, void* data);

    std::unique_ptr<EndpointMessage> sendRequest(const std::string &url, EndpointMessage &request, bool waitForResponse);

    // We purposely delete copy and assignment operators such that there isn't stray references to this
    ResourceDiscoveryConnEndpoint(ResourceDiscoveryConnEndpoint &) = delete;
    ResourceDiscoveryConnEndpoint &operator=(ResourceDiscoveryConnEndpoint &) = delete;

public:
    ResourceDiscoveryConnEndpoint(Component *component, SystemSchemas &ss) : component(component), systemSchemas(ss){}


    ///@{
    /**
     * @name RequestGenerators
     * @return The generated request
     */
    std::unique_ptr<EndpointMessage> generateRegisterRequest();
    std::unique_ptr<EndpointMessage> generateFindBySchemaRequest(const std::string &endpointType,
                                                               std::map<std::string, std::string> &otherValues);
    ///@}
    /**
     * Send a request to a ResourceDiscoveryHub at url and if it comes back successfully we register it as a Hub we can talk to.
     * Note that it doesn't throw exceptions, just outputs to cerr
     * @param url - The COMPLETE url of the RDH
     */
    void registerWithHub(const std::string &url);

    /**
     * Request ALL the IDs that a hub has.
     * @param url - The COMPLETE url of the RDH
     * @return Vector of IDs that hub has or an empty vector on error in contact
     */
    std::vector<std::string> getComponentIdsFromHub(const std::string &url);

    /**
     * Request the representation of a single component from an RDH
     * @param url - The COMPLETE url of the RDH
     * @param id - The id of the component we want
     * @return std::unique_ptr to a ComponentRepresentation from the RDH
     * @throws std::logic_error when the RDH doesn't reply properly or has not component
     */
    std::unique_ptr<ComponentRepresentation> getComponentById(const std::string &url, const std::string &id);



    /**
     * Sends requests to all the RDHs we know about, for the components which match the endpointType we request.
     * Assumes we only want DataReceiverEndpoints back
     * @param endpointType - Reference to the endpointType in our componentManifest
     * @return Vector of SocketMessages (which handle memory themselves) which follow Schema "SystemsSchemas/resource_discovery_by_schema_response"
     */
    std::vector<std::unique_ptr<EndpointMessage>>
    getComponentsBySchema(const std::string &endpointType, std::map<std::string, std::string> &otherValues);

    std::map<std::string, std::unique_ptr<ComponentRepresentation>>
    getComponentsByProperties(std::map<std::string, std::string> &properties);

    /**
     * Uses the getComponentsBySchema function to actually create connections to ALL of the available endpoints from our RDHs.
     * Should not throw any errors, we catch any creation errors. We do record in std::cerr when we can't connect to
     * any of the URLs for a component
     * @param endpointType - Reference to the endpointType in our ComponentManifest
     */
    void createEndpointBySchema(const std::string &endpointType);

    /**
     * Get our ID on the RDH represented
     * @param RdhUrl - RDH we want our ID for
     * @return ID
     * @throws std::out_of_range when we haven't registered with that RDH yet
     */
    std::string getId(const std::string &RdhUrl) {
        return resourceDiscoveryHubs.at(RdhUrl);
    }

    /**
     * Get the COMPLETE urls of all the RDHs we are connected to
     * @return Vector of complete urls (including port numbers)
     */
    std::vector<std::string> getResourceDiscoveryHubs() {
        std::vector<std::string> rdhs;
        for (auto url : resourceDiscoveryHubs) {
            rdhs.push_back(url.first);
        }
        return rdhs;
    }

    /**
     * Update our manifest with each RDH we are registered with
     */
    void updateManifestWithHubs();

    /**
     * Deregister a third party component from the given rdhUrl. This should be used largely for when we are unable to
     * contact a component and so deem that the component should be removed from the RDH
     */
    void deRegisterThirdPartyFromHub(const std::string &rdhUrl, const std::string componentId);

    /**
     * Deregister from the hub given. This means we no longer contact the hub
     */
    void deRegisterFromHub(const std::string &rdhUrl);

    /**
     * We search for resource discovery hubs by looking for components with background listeners. We then try to connect
     *  to the given RDHs, when we find 1 we stop as this is a slowwww process. We look for background listeners as each
     *  component has a background listener but there may just be one RDH on the network, so we are more likely
     *  to get a positive result
     */
    void searchForResourceDiscoveryHubs();

    /**
     * Deregister from the all the hubs we currently have connection to
     */
    void deRegisterFromAllHubs();

    /**
     * Tell all hubs that we have added a listenerPort for the given endpoint at port
     * @param endpointType - The endpoint type to update
     * @param port - The new port
     */
    void addListenerPortForAllHubs(const std::string& endpointType, int port);

    void checkLivenessOfHubs();

    static void asyncCheckHubLive(const std::string &url, bool *returnVal, ResourceDiscoveryConnEndpoint* rdc);
};


#endif //UBIFORM_RESOURCEDISCOVERYCONNENDPOINT_H
