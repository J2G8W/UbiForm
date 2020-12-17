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
#include "endpoints/PairEndpoint.h"
#include "endpoints/PublisherEndpoint.h"
#include "endpoints/SubscriberEndpoint.h"
#include "ResourceDiscovery/ResourceDiscoveryHubEndpoint.h"


class ResourceDiscoveryConnEndpoint;

class Component {
private:
    std::shared_ptr<ComponentManifest> componentManifest{nullptr};
    // Note that we use shared pointers so there can be multiple active pointers, but there memory management is handled automatically
    std::map<std::string, std::shared_ptr<DataReceiverEndpoint> > idReceiverEndpoints;
    std::map<std::string, std::shared_ptr<DataSenderEndpoint> > idSenderEndpoints;

    // Choice made to also store endpoints by TYPE, for speed of access and other things
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > > typeReceiverEndpoints;
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > > typeSenderEndpoints;

    nng_socket backgroundSocket;

    enum ComponentSystemSchema{manifestSchema, endpointCreationRequest, endpointCreationResponse};
    std::map<ComponentSystemSchema, std::unique_ptr<EndpointSchema>> systemSchemas;


    static void backgroundListen(Component *component);
    int lowestPort = 8000;
    std::thread backgroundThread;
    std::string backgroundListenAddress;


    static std::string requestConnection(const std::string& address, const std::string& requestText, size_t &sz);

public:
    Component();

    void specifyManifest(FILE *jsonFP) { componentManifest = std::make_shared<ComponentManifest>(jsonFP); }

    void specifyManifest(const char *jsonString) {
        componentManifest = std::make_shared<ComponentManifest>(jsonString);
    }

    void startBackgroundListen(const char *listenAddress);

    // We create a new Pair Endpoint and store it in our map as a SHARED pointer
    std::shared_ptr<PairEndpoint> createNewPairEndpoint(std::string type, std::string id);

    std::shared_ptr<SubscriberEndpoint> createNewSubscriberEndpoint(std::string type, std::string id);
    std::shared_ptr<PublisherEndpoint> createNewPublisherEndpoint(std::string type, std::string id);

    // We rethrow an out_of_range exception if the request fails
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string& id);
    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string& id);

    // No exception thrown here, return an empty vector if not found (which will get filled later on)
    std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > getReceiverEndpointsByType(const std::string &type);
    std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > getSenderEndpointsByType(const std::string &type);


    void requestPairConnection(const std::string& address, const std::string& endpointType);

    void requestConnectionToPublisher(const std::string& address, const std::string &endpointType);

    void startResourceDiscoveryHub(const std::string &listenAddress){
        ResourceDiscoveryHubEndpoint* rdh = new ResourceDiscoveryHubEndpoint;
        rdh->startResourceDiscover(listenAddress);
    }

    ResourceDiscoveryConnEndpoint* createResourceDiscoveryConnectionEndpoint();

    std::shared_ptr<ComponentManifest> getComponentManifest(){
        return componentManifest;
    }

    std::string & getBackgroundListenAddress(){
        return backgroundListenAddress;
    }

    ~Component();
};


#endif //UBIFORM_COMPONENT_H
