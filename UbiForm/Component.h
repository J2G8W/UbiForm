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
    std::string generateNewSocketId(){
        return std::to_string(generator());
    }
    int generateRandomPort(){
        return static_cast<int>(generator() % 60000 + 2000);
    }


    int lowestPort = 8001;

    std::string baseAddress;
    std::vector<std::string> availableAddresses;

    BackgroundListener backgroundListener;
    BackgroundRequester backgroundRequester;

    ResourceDiscoveryHubEndpoint * resourceDiscoveryHubEndpoint{nullptr};
    ResourceDiscoveryConnEndpoint resourceDiscoveryConnEndpoint;

    ConnectionType componentConnectionType;


public:
    explicit Component(const std::string & baseAddress);
    Component();

    void specifyManifest(FILE *jsonFP) {
        // TODO - close open connections?
        componentManifest.setManifest(jsonFP);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }
    void specifyManifest(const char *jsonString) {
        componentManifest.setManifest(jsonString);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }
    void specifyManifest(SocketMessage* sm) {
        componentManifest.setManifest(sm);
        resourceDiscoveryConnEndpoint.updateManifestWithHubs();
    }


    // We create a new Pair Endpoint and store it in our map as a SHARED pointer
    std::shared_ptr<PairEndpoint> createNewPairEndpoint(const std::string& type, const std::string& id);
    std::shared_ptr<SubscriberEndpoint> createNewSubscriberEndpoint(const std::string& type, const std::string& id);
    std::shared_ptr<PublisherEndpoint> createNewPublisherEndpoint(const std::string& type, const std::string& id);

    // Generalised start of listeners (returns URL of where connection is)
    int createEndpointAndListen(SocketType st, const std::string &endpointType);
    void createEndpointAndDial(const std::string &socketType, const std::string &localEndpointType, const std::string &url);

    // We rethrow an out_of_range exception if the request fails
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string& id);
    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string& id);

    // No exception thrown here, return an empty vector if not found (which will get filled later on)
    std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > getReceiverEndpointsByType(const std::string &endpointType);
    std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > getSenderEndpointsByType(const std::string &endpointType);


    void startBackgroundListen(int port);
    void startBackgroundListen();

    void startResourceDiscoveryHub(int port);
    int startResourceDiscoveryHub();


    ResourceDiscoveryConnEndpoint & getResourceDiscoveryConnectionEndpoint(){return resourceDiscoveryConnEndpoint;}
    ComponentManifest& getComponentManifest(){return componentManifest;}
    SystemSchemas & getSystemSchemas(){return systemSchemas;}
    BackgroundRequester & getBackgroundRequester(){return backgroundRequester;}
    std::string getRDHLocation();

    int getBackgroundPort(){return backgroundListener.getBackgroundPort();}
    std::vector<std::string>& getAllAddresses(){
        return availableAddresses;
    }


    void closeSocketsOfType(const std::string& endpointType);

    ~Component();
};


#endif //UBIFORM_COMPONENT_H
