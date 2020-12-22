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
#include "SystemSchemas/SystemSchemas.h"


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

    SystemSchemas systemSchemas;

    std::minstd_rand0 generator;
    std::string generateNewSocketId(){
        return std::to_string(generator());
    }
    int generateRandomPort(){
        return static_cast<int>(generator() % 60000 + 2000);
    }

    static void backgroundListen(Component *component);
    int lowestPort = 8001;
    std::thread backgroundThread;
    std::string backgroundListenAddress;

    std::string baseAddress;

    ResourceDiscoveryConnEndpoint * resourceDiscoveryConnEndpoint {nullptr};

    static std::string requestConnection(const std::string& address, const std::string& requestText);

public:
    explicit Component(const std::string & baseAddress);
    Component() : Component("tcp://127.0.0.1"){}

    void specifyManifest(FILE *jsonFP) {
        componentManifest = std::make_shared<ComponentManifest>(jsonFP,systemSchemas);
    }

    void specifyManifest(const char *jsonString) {
        componentManifest = std::make_shared<ComponentManifest>(jsonString,systemSchemas);
    }

    void startBackgroundListen(int port);
    void startBackgroundListen();

    // We create a new Pair Endpoint and store it in our map as a SHARED pointer
    std::shared_ptr<PairEndpoint> createNewPairEndpoint(const std::string& type, const std::string& id);

    std::shared_ptr<SubscriberEndpoint> createNewSubscriberEndpoint(const std::string& type, const std::string& id);
    std::shared_ptr<PublisherEndpoint> createNewPublisherEndpoint(const std::string& type, const std::string& id);

    // We rethrow an out_of_range exception if the request fails
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpointById(const std::string& id);
    std::shared_ptr<DataSenderEndpoint> getSenderEndpointById(const std::string& id);

    // No exception thrown here, return an empty vector if not found (which will get filled later on)
    std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > getReceiverEndpointsByType(const std::string &endpointType);
    std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > getSenderEndpointsByType(const std::string &endpointType);

    void requestAndCreateConnection(const std::string& localEndpointType, const std::string &connectionComponentAddress,
                                    const std::string &remoteEndpointType);



    void startResourceDiscoveryHub(int port);

    ResourceDiscoveryConnEndpoint* createResourceDiscoveryConnectionEndpoint();

    std::shared_ptr<ComponentManifest> getComponentManifest(){return componentManifest;}

    std::string & getBackgroundListenAddress(){return backgroundListenAddress;}

    ~Component();
};


#endif //UBIFORM_COMPONENT_H
