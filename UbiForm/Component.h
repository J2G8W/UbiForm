#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"

#include <iostream>
#include <memory>
#include <map>
#include <thread>

#include "ComponentManifest.h"
#include "SocketMessage.h"
#include "Endpoints/PairEndpoint.h"
#include "Endpoints/PublisherEndpoint.h"
#include "Endpoints/SubscriberEndpoint.h"
#include "ResourceDiscovery/ResourceDiscoveryHubEndpoint.h"
#include "SystemSchemas/SystemSchemas.h"
#include "ReconfigEndpoints/BackgroundListener.h"
#include "ReconfigEndpoints/BackgroundRequester.h"
#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#define DEFAULT_BACKGROUND_LISTEN_PORT 8000
#define DEFAULT_RESOURCE_DISCOVERY_PORT 7999


class Component {
private:
    SystemSchemas systemSchemas;

    ComponentManifest componentManifest;
    // Note that we use shared pointers so there can be multiple active pointers, but there memory management is handled automatically
    std::map<std::string, std::shared_ptr<DataReceiverEndpoint> > idReceiverEndpoints;
    std::map<std::string, std::shared_ptr<DataSenderEndpoint> > idSenderEndpoints;

    // Choice made to also store endpoints by TYPE, for speed of access and other things
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > > typeReceiverEndpoints;
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > > typeSenderEndpoints;


    std::minstd_rand0 generator;

    std::string generateNewSocketId() {
        return std::to_string(generator());
    }

    int generateRandomPort() {
        return static_cast<int>(generator() % 60000 + 2000);
    }


    int lowestPort = 8001;

    std::string selfAddress;
    std::vector<std::string> availableAddresses;

    BackgroundListener backgroundListener;
    BackgroundRequester backgroundRequester;

    ResourceDiscoveryHubEndpoint *resourceDiscoveryHubEndpoint{nullptr};
    ResourceDiscoveryConnEndpoint resourceDiscoveryConnEndpoint;

    ConnectionType componentConnectionType;


public:
    /**
     * Create a Component object which will specifically be LocalTCP or IPC, must start with "ipc://" or "tcp://127.".
     * This is designed to be used for testing purposes largely
     * @param baseAddress - address to listen on
     */
    explicit Component(const std::string &baseAddress);

    /**
     * Create a component which will listen on ALL external IPv4 connections
     * @throws std::logic_error when can't find any valid external IPv4 connections to join on to
     */
    Component();

    /** Specifies the manifest of the component (will overwrite previous manifest if one exists).
     * All constructors are copy constructors from their location */
    ///@{
    void specifyManifest(FILE *jsonFP) {
        // TODO - close open connections?
        componentManifest.setManifest(jsonFP);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }

    void specifyManifest(const char *jsonString) {
        componentManifest.setManifest(jsonString);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }

    void specifyManifest(SocketMessage *sm) {
        componentManifest.setManifest(sm);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }
    ///@}



    /**
     *
     * @param - Type refers to an identifier in the componentManifest
     * @param - Id is a unique identifier given to the new endpoint within the component
     */
    void createNewEndpoint(const std::string &type, const std::string &id);


    /**
     * Creates an endpoint of socketType which refers to the endpointType in the componentManifest. It then listens for
     * incoming connections.
     * @param st - Specify what type of connection is created (Pair, Publisher etc)
     * @param endpointType - Specifies what type of connection is created (refers to componentManifest)
     * @return The port number which the endpoint is listening on
     */
    int createEndpointAndListen(SocketType st, const std::string &endpointType);

    /**
     * Creates and dials
     * @param socketType - Specify what type of connection is created (Pair, Publisher etc)
     * @param localEndpointType - Refers to our own componentManifest
     * @param url - The complete URL to listen on (form tcp://_._._._:_)
     */
    void
    createEndpointAndDial(const std::string &socketType, const std::string &localEndpointType, const std::string &url);

    ///@{
    /// Get a pointer to endpoints, they are shared_ptr's which shouldn't be deleted, and may be closed without notice
    /// @throws std::out_of_range if the id does not appear in our OPEN endpoints
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string &id);

    /// @throws std::out_of_range if the id does not appear in our OPEN endpoints
    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string &id);
    ///@}

    ///@{
    /// Get pointer to a vector of endpoints, again these will be manipulated without notice and will be added to and
    /// emptied as we go. Does not throw any access error for an endpoint type, but returns an empty vector which is filled
    /// if things of that type are created
    std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > >
    getReceiverEndpointsByType(const std::string &endpointType);

    std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > >
    getSenderEndpointsByType(const std::string &endpointType);
    ///@}


    ///@{
    /**
     * Start a background listener process for the component which will handle requests for reconfiguration of the component.
     * You can only have one background listener at a time.
     * @param port - specific port to listen to
     * @throws NngError if there is an error listening on that port
     */
    void startBackgroundListen(int port);

    /**
     * @throws std::logic_error if we can't find a valid port on 5 attempts at randomisation
     */
    void startBackgroundListen();
    ///@}

    ///@{
    /**
     * Start a ResourceDiscoveryHub process for the component which will store other components on the network and handle requests.
     * You can only have on RDH at a time
     * @param port
     */
    void startResourceDiscoveryHub(int port);

    int startResourceDiscoveryHub();
    ///@}

    void closeResourceDiscoveryHub();

    std::string getSelfAddress() { return selfAddress; }

    int getResourceDiscoveryHubPort();


    ResourceDiscoveryConnEndpoint &getResourceDiscoveryConnectionEndpoint() { return resourceDiscoveryConnEndpoint; }

    ComponentManifest &getComponentManifest() { return componentManifest; }

    SystemSchemas &getSystemSchemas() { return systemSchemas; }

    BackgroundRequester &getBackgroundRequester() { return backgroundRequester; }

    ConnectionType getComponentConnectionType() { return componentConnectionType; }

    int getBackgroundPort() { return backgroundListener.getBackgroundPort(); }

    /// @brief - Returns a reference to the vector of all the addresses that the component can listen on
    std::vector<std::string> &getAllAddresses() {
        return availableAddresses;
    }

    /**
     * Close the sockets of endpointType. The vector which represents the endpointType is emptied and if users haven't got pointers
     * the endpoints will be deleted. If the users to do have their own pointers, then we set the endpoints to closed and they throw exception
     * when asked to do anything
     * @param endpointType
     */
    void closeSocketsOfType(const std::string &endpointType);


    void closeSocketOfId(const std::string &endpointId);

    /// Pretty much everything should stop once component is deleted
    ~Component();
};


#endif //UBIFORM_COMPONENT_H
