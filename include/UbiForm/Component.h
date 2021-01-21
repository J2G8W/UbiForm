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
#include "../../UbiForm/SystemSchemas/SystemSchemas.h"
#include "ReconfigurationEndpoints/BackgroundListener.h"
#include "ReconfigurationEndpoints/BackgroundRequester.h"
#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#define DEFAULT_BACKGROUND_LISTEN_PORT 8000
#define DEFAULT_RESOURCE_DISCOVERY_PORT 7999


class Component {
public:
    typedef void (*startupFunc)(std::shared_ptr<DataReceiverEndpoint>,std::shared_ptr<DataSenderEndpoint>, void *);
private:
    SystemSchemas systemSchemas;

    ComponentManifest componentManifest;

    std::map<std::string, startupFunc> startupFunctionsMap;
    std::map<std::string, void*> startupDataMap;

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

    /**
     * @name Specify Manifests
     * Specifies the manifest of the component (will overwrite previous manifest if one exists).
     * All constructors are copy constructors from their location and close ALL connected endpoints on update and will
     * update any relevant RDHs once done*/
    ///@{
    void specifyManifest(FILE *jsonFP);

    void specifyManifest(const char *jsonString);

    void specifyManifest(SocketMessage *sm);
    ///@}



    /**
     *
     * @param - Type refers to an identifier in the componentManifest
     * @param - Id is a unique identifier given to the new endpoint within the component
     */
    void createNewEndpoint(const std::string &endpointType, const std::string &endpointId);


    /**
     * Creates an endpoint of socketType which refers to the endpointType in the componentManifest. It then listens for
     * incoming connections.
     * @param endpointType - Specifies what type of connection is created (refers to componentManifest)
     * @return The port number which the endpoint is listening on
     */
    int createEndpointAndListen(const std::string &endpointType);

    /**
     * Creates and dials
     * @param localEndpointType - Refers to our own componentManifest
     * @param dialUrl - The complete URL to listen on (form tcp://_._._._:_)
     */
    void
    createEndpointAndDial(const std::string &localEndpointType, const std::string &dialUrl);


    ///@{
    /**
     * @brief Get a pointer to endpoints, they are shared_ptr's which shouldn't be deleted, and may be closed without notice
     * @name Get Endpoints by ID
     * @throws std::out_of_range if the id does not appear in our OPEN endpoints
     * @param id - The local id of the endpoint we want
     * @return A shared_ptr to an endpoint, note that this may be in an "Invalid" state at some, at which point any actions throw SocketOpenError
    **/
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string &id);

    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string &id);
    ///@}


    ///@{
    /**
     * @brief Get pointer to a vector of endpoints, again these will be manipulated without notice and will be added to and
     * emptied as we go. Does not throw any access error for an endpoint type, but returns an empty vector which is filled
     * if things of that type are created
     * @name Get Endpoint by type
     * @param endpointType - The type of endpoint we want (referenced in the ComponentManifest)
     * @return A pointer to a vector of endpoint (we say a pointer so that we can manipulate the vector in BackgroundListener)
     *
     */
    std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > >
    getReceiverEndpointsByType(const std::string &endpointType);

    std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > >
    getSenderEndpointsByType(const std::string &endpointType);
    ///@}



    ///@{
    /**
     * @name Start Background Listen
     * @brief Start a background listener process for the component which will handle requests for reconfiguration of the component.
     * You can only have one background listener at a time.
     * @param port - specific port to listen to
     * @throws NngError if there is an error listening on that port
     */
    void startBackgroundListen(int port);

    void startBackgroundListen();
    ///@}

    ///@{
    /**
     * @name Start Resource Discovery Hub
     * Start a ResourceDiscoveryHub process for the component which will store other components on the network and handle requests.
     * You can only have on RDH at a time
     * @param port
     */
    void startResourceDiscoveryHub(int port);

    int startResourceDiscoveryHub();
    ///@}

    /**
     * Close the Resource Discovery Hub attached to the component. If there is no RDH then we don't do anything
     */
    void closeResourceDiscoveryHub();

    /**
     * @return The self address is the address by which the componet can reach itself
     */
    std::string getSelfAddress() { return selfAddress; }

    /**
     * @return The port that the RDH is on
     * @throws std::logic_error - when there is no RDH started
     */
    int getResourceDiscoveryHubPort();


    /// @return A reference to our Resource Discovery Connection Endpoint, from which we make RDH requests
    ResourceDiscoveryConnEndpoint &getResourceDiscoveryConnectionEndpoint() { return resourceDiscoveryConnEndpoint; }

    /// @return A reference to the Component Manifest which we can change/read. Note that changes to manifest may need "updateManifestWithHubs()"
    ComponentManifest &getComponentManifest() { return componentManifest; }

    /// @return A reference to the SystemSchemas object we have. The Component should have the only copy of the SystemSchemas
    SystemSchemas &getSystemSchemas() { return systemSchemas; }

    /// @return A reference to our Background Requester so we can make requests to other parties
    BackgroundRequester &getBackgroundRequester() { return backgroundRequester; }

    /// @return The type of connection our Component makes (either TCP, LocalTCP or IPC)
    ConnectionType getComponentConnectionType() { return componentConnectionType; }

    /// @return The background port of our listener. Set to -1 if there is no Background Listener
    int getBackgroundPort() { return backgroundListener.getBackgroundPort(); }

    /// @brief - Returns a reference to the vector of all the addresses that the component can listen on
    std::vector<std::string> &getAllAddresses() {
        return availableAddresses;
    }

    /**
     * Close the sockets of endpointType. The vector which represents the endpointType is emptied and if users haven't got pointers
     * the endpoints will be deleted. If the users to do have their own pointers, then we set the endpoints to INVALID and they throw exception
     * when asked to do anything
     * @param endpointType
     */
    void closeAndInvalidateSocketsOfType(const std::string &endpointType);

    /**
     * Close by id. Makes the given endpoint Invalid, does nothing if the endpointID does not exist
     * @param endpointId
     */
    void closeAndInvalidateSocketById(const std::string &endpointId);

    /**
     * Close all the used made sockets on our component (apart from pre-defined sockets)
     */
    void closeAndInvalidateAllSockets();

    /// Pretty much everything should stop once component is deleted
    ~Component();

    void registerStartupFunction(const std::string &endpointType, startupFunc startupFunction, void *startupData);
};


#endif //UBIFORM_COMPONENT_H
