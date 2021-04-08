#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"

#include <iostream>
#include <memory>
#include <map>
#include <thread>

#include "Utilities/ExceptionClasses.h"
#include "ComponentManifest.h"
#include "EndpointMessage.h"
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


/**
 * The central concept to UbiForm. Each process should only need one component at any point and all network operations should
 * go via the component (using the getters we access the actual function we want). The component is also used for creation,
 * access and destruction of endpoints and controls the memory access of these objects.
 */
class Component {
private:
    SystemSchemas systemSchemas;

    ComponentManifest componentManifest;

    std::map<std::string, endpointStartupFunction> startupFunctionsMap;
    std::map<std::string, void*> startupDataMap;

    std::map<std::string, std::shared_ptr<Endpoint>> endpointsById;

    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<Endpoint> > >> endpointsByType;


    std::minstd_rand0 generator;

    std::string generateNewEndpointId() {
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
     * @brief Create a Component object which will specifically be LocalTCP or IPC (designed to be used for testing purposes largely)
     * @param baseAddress - Address to listen on (must start with tcp://127 or ipc:///
     */
    explicit Component(const std::string &baseAddress);

    /**
     * @brief Create a component which will listen on ALL external IPv4 connections to the device
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

    void specifyManifest(EndpointMessage *sm);
    ///@}



    /**
     * @name Create Endpoints
     * Functions to create endpoints and do different initialisation work for them
     * @{
     */
    /**
     * @brief Create a new endpoint of endpointType (refers to manifest) with endpointId
     * @param - Type refers to an identifier in the componentManifest
     * @param - Id is a unique identifier given to the new endpoint within the component
     */
    void createNewEndpoint(const std::string &endpointType, const std::string &endpointId);


    /**
     * @brief Creates an endpoint of endpointType (refers to manifest), then listens for incoming connections.
     * @param endpointType - Specifies what type of connection is created (refers to manifest)
     * @return The port number which the endpoint is listening on
     */
    int createEndpointAndListen(const std::string &endpointType);

    /**
     * @brief Creates an endpoint and dials
     * @param localEndpointType - Refers to our own componentManifest
     * @param dialUrl - The complete URL to listen on (form tcp://_._._._:_)
     */
    void
    createEndpointAndDial(const std::string &localEndpointType, const std::string &dialUrl);
    ///@}


    ///@{
    /**
     * @name Get Endpoints by ID
     * Get a shared pointer to endpoints based on the provided ID
     *
     * @throws std::out_of_range if the id does not appear in our OPEN endpoints
     * @param id - The local id of the endpoint we want
     * @return A shared_ptr to an endpoint, note that this may be in an "Invalid" state at some, at which point any actions throw EndpointOpenError
    **/
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string &id);

    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string &id);

    std::shared_ptr<Endpoint> getEndpointById(const std::string& id);
    ///@}


    /**
     * @brief Get a shared pointer to a vector of endpoints which are all of endpointType
     * @param endpointType - The type of endpoint we want (referenced in the ComponentManifest)
     * @return A pointer to a vector of endpoint (we say a pointer so that we can manipulate the vector in BackgroundListener)
     *
     * These will be manipulated without notice and will be added to and
     * emptied as we go. Does not throw any access error for an endpoint type, but returns an empty vector which is filled
     * if things of that type are created
     */
    std::shared_ptr<std::vector<std::shared_ptr<Endpoint>>> getEndpointsByType(const std::string &endpointType);




    ///@{
    /**
     * @name Start Background Listen
     * Start background listen process
    **/
    void startBackgroundListen(int port);

    void startBackgroundListen();
    ///@}

    ///@{
    /**
     * @name Start Resource Discovery Hub
     * Start a ResourceDiscoveryHub process for the component which will store other components on the network and handle requests.
     * You can only have on RDH at a time
     * @param port - specific port to listen on
     */
    void startResourceDiscoveryHub(int port);

    int startResourceDiscoveryHub();
    ///@}

    /**
     * @brief Close the Resource Discovery Hub attached to the component. If there is no RDH then we don't do anything
     */
    void closeResourceDiscoveryHub();

    /**
     * @brief Get the address by which the component can reach itself
     * @return The self address is the address by which the componet can reach itself
     */
    std::string getSelfAddress() { return selfAddress; }

    /**
     * @brief Get the port for the resource discovery hub of this component
     * @return The port that the RDH is on
     * @throws std::logic_error - when there is no RDH started
     */
    int getResourceDiscoveryHubPort();

    /**
     * @brief Directly access the resource discovery hub attached to this component
     * @return The connections of the RDH
     */
    std::vector<std::shared_ptr<ComponentRepresentation>> getResourceDiscoveryHubConnections();


    /// @brief Return a reference to our Resource Discovery Connection Endpoint, from which we make RDH requests
    ResourceDiscoveryConnEndpoint &getResourceDiscoveryConnectionEndpoint() { return resourceDiscoveryConnEndpoint; }

    /// @brief Return a reference to the Component Manifest which we can change/read. Note that changes to manifest may need "updateManifestWithHubs()"
    ComponentManifest &getComponentManifest() { return componentManifest; }

    /// @brief Return a reference to the SystemSchemas object we have. The Component should have the only copy of the SystemSchemas
    SystemSchemas &getSystemSchemas() { return systemSchemas; }

    /// @brief Return a reference to our Background Requester so we can make requests to other parties
    BackgroundRequester &getBackgroundRequester() { return backgroundRequester; }

    /// @brief Return the type of connection our Component makes (either TCP, LocalTCP or IPC)
    ConnectionType getComponentConnectionType() { return componentConnectionType; }

    /// @brief Return the background port of our listener. Set to -1 if there is no Background Listener
    int getBackgroundPort() { return backgroundListener.getBackgroundPort(); }

    /// @brief Return a reference to the vector of all the addresses that the component can listen on
    std::vector<std::string> &getAllAddresses() {
        return availableAddresses;
    }

    /**
     * @brief Close the endpoints of endpointType.
     *
     * The vector which represents the endpointType is emptied and if users haven't got pointers
     * the endpoints will be deleted. If the users to do have their own pointers, then we set the endpoints to INVALID and they throw exception
     * when asked to do anything
     * @param endpointType
     */
    void closeAndInvalidateEndpointsOfType(const std::string &endpointType);

    /**
     * @brief Close endpoint by id. Makes the given endpoint Invalid, does nothing if the endpointID does not exist
     * @param endpointId
     */
    void closeAndInvalidateEndpointsById(const std::string &endpointId);

    /**
     * @brief Close all the used made endpoints on our component (apart from pre-defined endpoints)
     */
    void closeAndInvalidateAllEndpoints();


    /**
     * @brief This destructor takes some time as it gracefully closes each endpoint. This requires a short period of time as we
     * need to a) Close the communications and b) Join any background thread that was running
     */
    ~Component();

    /**
     * @brief This can be used to run the given function whenever the given endpoint is CONNECTED (that is it dials or listens).
     * It runs as a std::thread in the background and is cleanly handled IF the given the function does
     * not block on non-network calls.
     *
     * Additionally the function should not throw any exceptions as these are incredibly
     * difficult to diagnose, so the suggestion is to wrap any work in try/catch for good debugging
     * If there is not an endpointType in our manifest then this function does nothing and if there is already a function
     * registered then we overwrite this.
     * @param endpointType - The type of endpoint we want to attach to
     * @param startupFunction - The function we want to call whenever the endpoint connects. It receives a pointer
     * to the created endpoint and the startupData which is provided
     * @param startupData - Some arbitrary data which is passed as extra to the startup function (can be nullptr)
     */
    void registerStartupFunction(const std::string &endpointType, endpointStartupFunction startupFunction, void *startupData);


    /**
     * @name Casting operations
     * We give the ability for an endpoint to be dynamically casted to the given type. We check that there shouldn't be an
     * error on casting by looking at our manifest and endpoint ConnectionParadigm
     * @throws AccessError If the endpoint is the wrong type
     */
    ///{
    PairEndpoint* castToPair(Endpoint *e);
    std::shared_ptr<PairEndpoint> castToPair(std::shared_ptr<Endpoint> e);

    std::shared_ptr<DataReceiverEndpoint> castToDataReceiverEndpoint(std::shared_ptr<Endpoint>);
    DataReceiverEndpoint* castToDataReceiverEndpoint(Endpoint*);

    std::shared_ptr<DataSenderEndpoint> castToDataSenderEndpoint(std::shared_ptr<Endpoint>);
    DataSenderEndpoint* castToDataSenderEndpoint(Endpoint*);
    ///}
};


#endif //UBIFORM_COMPONENT_H
