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

class Component {
private:
    std::unique_ptr<ComponentManifest> componentManifest{nullptr};
    // Note that we use shared pointers so there can be multiple active pointers, but there memory management is handled automatically
    std::map<std::string, std::shared_ptr<DataReceiverEndpoint> > idReceiverEndpoints;
    std::map<std::string, std::shared_ptr<DataSenderEndpoint> > idSenderEndpoints;

    // Choice made to also store endpoints by TYPE, for speed of access and other things
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > > > typeReceiverEndpoints;
    std::map<std::string, std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > > > typeSenderEndpoints;

    nng_socket backgroundSocket;

    static void backgroundListen(Component *component);
    int lowestPort = 8000;
    std::thread backgroundThread;


    static char* requestConnection(const std::string& address, const std::string& requestText);

public:
    Component() = default;

    void specifyManifest(FILE *jsonFP) { componentManifest = std::make_unique<ComponentManifest>(jsonFP); }

    void specifyManifest(const char *jsonString) {
        componentManifest = std::make_unique<ComponentManifest>(jsonString);
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

    ~Component();
};


#endif //UBIFORM_COMPONENT_H
