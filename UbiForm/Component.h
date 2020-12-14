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

class Component {
private:
    std::unique_ptr<ComponentManifest> componentManifest{nullptr};
    // Note that we use shared pointers so there can be multiple active pointers, but there memory management is handled automatically
    std::map<std::string, std::shared_ptr<DataReceiverEndpoint> > receiverEndpoints;
    std::map<std::string, std::shared_ptr<DataSenderEndpoint> > senderEndpoints;

    nng_socket backgroundSocket;

    static void backgroundListen(Component *component);
    int lowestPort = 8000;
    std::thread backgroundThread;


public:
    Component() = default;

    void specifyManifest(FILE *jsonFP) { componentManifest = std::make_unique<ComponentManifest>(jsonFP); }

    void specifyManifest(const char *jsonString) {
        componentManifest = std::make_unique<ComponentManifest>(jsonString);
    }

    void startBackgroundListen();

    // We create a new Pair Endpoint and store it in our map as a SHARED pointer
    std::shared_ptr<PairEndpoint> createNewPairEndpoint(std::string type, std::string id);

    void createNewSubscriberEndpoint(std::string type, std::string id);
    void createNewPublisherEndpoint(std::string type, std::string id);

    // We rethrow an out_of_range exception if the request fails
    // shared pointer is returned for C++ ness
    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpoint(const std::string& id);;

    // We rethrow an out_of_range exception if the request fails
    std::shared_ptr<DataSenderEndpoint> getSenderEndpoint(const std::string& id);;


    void requestPairConnection(const std::string& address, const std::string& endpointType);

    ~Component();
};


#endif //UBIFORM_COMPONENT_H
