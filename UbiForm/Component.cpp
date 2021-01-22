
#include "../include/UbiForm/Component.h"
#include "../include/UbiForm/Utilities/SystemEnums.h"

#include "../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
#include "Utilities/GetIPAddress.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>


// CONSTRUCTOR
Component::Component(const std::string &baseAddress) : systemSchemas(),
                                                       backgroundListener(this, systemSchemas),
                                                       backgroundRequester(this, systemSchemas),
                                                       selfAddress(baseAddress),
                                                       resourceDiscoveryConnEndpoint(this, systemSchemas),
                                                       componentManifest(systemSchemas) {
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);
    std::cout << "Component made, self address is " << baseAddress << std::endl;
    if (baseAddress.rfind("tcp", 0) == 0) {
        componentConnectionType = ConnectionType::LocalTCP;
        availableAddresses.push_back(baseAddress);
    } else if (baseAddress.rfind("ipc", 0) == 0) {
        componentConnectionType = ConnectionType::IPC;
        availableAddresses.push_back(baseAddress);
    } else {
        throw std::logic_error("The given address did not have a type of TCP or IPC");
    }
}

Component::Component() : systemSchemas(),
                         backgroundListener(this, systemSchemas), backgroundRequester(this, systemSchemas),
                         resourceDiscoveryConnEndpoint(this, systemSchemas), componentManifest(systemSchemas) {
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);

    auto addresses = getIpAddresses();
    for (const auto &address: addresses) {
        if (address.rfind("127.", 0) != 0) {
            availableAddresses.emplace_back("tcp://" + address);
        }
    }
    if (availableAddresses.empty()) {
        throw std::logic_error("Could not find any networks to act on");
    } else {
        std::cout << "Available IP addresses: " << std::endl;
        for (const auto &address : availableAddresses) {
            std::cout << "\t" << address << std::endl;
        }
    }
    selfAddress = "tcp://127.0.0.1";
    componentConnectionType = ConnectionType::TCP;
    std::cout << "Component made, self address is " << selfAddress << std::endl;
}


void Component::specifyManifest(FILE *jsonFP) {
    closeAndInvalidateAllSockets();
    componentManifest.setManifest(jsonFP);
    resourceDiscoveryConnEndpoint.updateManifestWithHubs();
}

void Component::specifyManifest(const char *jsonString) {
    closeAndInvalidateAllSockets();
    componentManifest.setManifest(jsonString);
    resourceDiscoveryConnEndpoint.updateManifestWithHubs();
}

void Component::specifyManifest(SocketMessage *sm) {
    closeAndInvalidateAllSockets();
    componentManifest.setManifest(sm);
    resourceDiscoveryConnEndpoint.updateManifestWithHubs();
}


void Component::createNewEndpoint(const std::string &endpointType, const std::string &endpointId) {
    SocketType socketType = convertToSocketType(componentManifest.getSocketType(endpointType));

    std::shared_ptr<EndpointSchema> recvSchema;
    std::shared_ptr<EndpointSchema> sendSchema;
    if (socketType == SocketType::Pair || socketType == SocketType::Subscriber || socketType == SocketType::Request ||
        socketType == SocketType::Reply) {
        recvSchema = componentManifest.getReceiverSchema(endpointType);
    }
    if (socketType == SocketType::Pair || socketType == SocketType::Publisher || socketType == SocketType::Request ||
        socketType == SocketType::Reply) {
        sendSchema = componentManifest.getSenderSchema(endpointType);
    }

    std::shared_ptr<DataReceiverEndpoint> receiverEndpoint;
    std::shared_ptr<DataSenderEndpoint> senderEndpoint;

    endpointStartupFunction startupFunc = nullptr;
    void* extraData = nullptr;
    if(startupFunctionsMap.count(endpointType)){
        startupFunc = startupFunctionsMap.at(endpointType);
        if(startupDataMap.count(endpointType)){
            extraData = startupDataMap.at(endpointType);
        }
    }

    switch (socketType) {
        case Pair: {
            auto pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema, endpointType, endpointId, startupFunc,extraData);
            receiverEndpoint = pe;
            senderEndpoint = pe;
            break;
        }
        case Publisher:
            senderEndpoint = std::make_shared<PublisherEndpoint>(sendSchema, endpointType, endpointId, startupFunc, extraData);
            break;
        case Subscriber:
            receiverEndpoint = std::make_shared<SubscriberEndpoint>(recvSchema, endpointType, endpointId, startupFunc, extraData);
            break;
        case Reply: {
            auto pe = std::make_shared<ReplyEndpoint>(recvSchema, sendSchema, endpointType, endpointId, startupFunc, extraData);
            receiverEndpoint = pe;
            senderEndpoint = pe;
            break;
        }
        case Request: {
            auto pe = std::make_shared<RequestEndpoint>(recvSchema, sendSchema, endpointType, endpointId, startupFunc, extraData);
            receiverEndpoint = pe;
            senderEndpoint = pe;
            break;
        }
    }



    if (socketType == SocketType::Pair || socketType == SocketType::Subscriber || socketType == SocketType::Request ||
        socketType == SocketType::Reply) {
        idReceiverEndpoints.insert(std::make_pair(endpointId, receiverEndpoint));
        if (typeReceiverEndpoints.count(endpointType) == 1) {
            typeReceiverEndpoints.at(endpointType)->push_back(receiverEndpoint);
        } else {
            typeReceiverEndpoints.insert(std::make_pair(
                    endpointType,
                    std::make_shared<std::vector<std::shared_ptr<DataReceiverEndpoint> > >(1, receiverEndpoint)));
        }
    }
    if (socketType == SocketType::Pair || socketType == SocketType::Publisher || socketType == SocketType::Request ||
        socketType == SocketType::Reply) {
        idSenderEndpoints.insert(std::make_pair(endpointId, senderEndpoint));
        if (typeSenderEndpoints.count(endpointType) == 1) {
            typeSenderEndpoints.at(endpointType)->push_back(senderEndpoint);
        } else {
            typeSenderEndpoints.insert(std::make_pair(endpointType,
                                                      std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >(
                                                              1, senderEndpoint)));
        }
    }
}


// GET OUR ENDPOINTS BY ID
std::shared_ptr<DataReceiverEndpoint> Component::getReceiverEndpointById(const std::string &id) {
    try {
        return idReceiverEndpoints.at(id);
    } catch (std::out_of_range &e) {
        throw;
    }
}

std::shared_ptr<DataSenderEndpoint> Component::getSenderEndpointById(const std::string &id) {
    try {
        return idSenderEndpoints.at(id);
    } catch (std::out_of_range &e) {
        throw;
    }
}

// GET OUR ENDPOINTS BY ENDPOINTTYPE
std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > >
Component::getReceiverEndpointsByType(const std::string &endpointType) {
    try {
        return typeReceiverEndpoints.at(endpointType);
    } catch (std::out_of_range &e) {
        // Make an empty vector to return, this will get filled if things then come along later
        auto returnVector = std::make_shared<std::vector<std::shared_ptr<DataReceiverEndpoint> > >();
        typeReceiverEndpoints.insert(std::make_pair(endpointType, returnVector));
        return returnVector;
    }
}


std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > >
Component::getSenderEndpointsByType(const std::string &endpointType) {
    try {
        return typeSenderEndpoints.at(endpointType);
    } catch (std::out_of_range &e) {
        // Make an empty vector to return, this will get filled if things then come along later
        auto returnVector = std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >();
        typeSenderEndpoints.insert(std::make_pair(endpointType, returnVector));
        return returnVector;
    }
}

// THIS IS OUR COMPONENT LISTENING FOR REQUESTS TO MAKE SOCKETS
void Component::startBackgroundListen() {
    // -1 is the initalise value, and signifies the backgroundListener ain't doing nothing yet
    if (backgroundListener.getBackgroundPort() == -1) {
        int nextPort = DEFAULT_BACKGROUND_LISTEN_PORT;
        for (int i = 0; i < 5; i++) {
            try {
                startBackgroundListen(nextPort);
                return;
            } catch (NngError &e) {
                if (e.errorCode == NNG_EADDRINUSE) {
                    this->lowestPort = generateRandomPort();
                    nextPort = this->lowestPort;
                } else {
                    throw;
                }
            }
        }
        throw std::logic_error("Could not find valid port to start on");
    }
}

void Component::startBackgroundListen(int port) {
    if (backgroundListener.getBackgroundPort() == -1) {
        if (componentConnectionType == ConnectionType::TCP) {
            // Listen on all addresses
            backgroundListener.startBackgroundListen("tcp://*", port);
        } else {
            // Listen on just the address given (either local or IPC)
            backgroundListener.startBackgroundListen(selfAddress, port);
        }
    }
}

int Component::createEndpointAndListen(const std::string &endpointType) {
    std::string socketId = generateNewSocketId();
    std::shared_ptr<DataSenderEndpoint> e;
    // If we already have a publisher or reply endpoint already then we don't want to make a new one
    if(componentManifest.getSocketType(endpointType) == PUBLISHER || componentManifest.getSocketType(endpointType) == REPLY){
        if(getSenderEndpointsByType(endpointType)->empty()){
            createNewEndpoint(endpointType,socketId);
        } else {
            EndpointState senderState = getSenderEndpointsByType(endpointType)->at(0)->getEndpointState();
            switch (senderState) {
                case Closed:
                    getSenderEndpointsByType(endpointType)->at(0)->openEndpoint();
                    break;
                case Open:
                case Listening:
                    socketId = getSenderEndpointsByType(endpointType)->at(0)->getEndpointId();
                    break;
                default:
                    createNewEndpoint(endpointType,socketId);
            }
        }
    }else{
        createNewEndpoint(endpointType, socketId);
    }

    try {
        e = getSenderEndpointById(socketId);
    } catch (std::out_of_range &e) {
        throw std::logic_error(
                "Couldn't make endpoint of type " + endpointType);
    }

    if(e->getEndpointState() != EndpointState::Listening) {
        int rv = 1;
        std::string url;
        while (rv != 0) {
            if (componentConnectionType == ConnectionType::TCP) {
                url = "tcp://*";
            } else {
                url = selfAddress;
            }
            rv = e->listenForConnectionWithRV(url.c_str(), lowestPort);
            if (rv == NNG_EADDRINUSE) {
                lowestPort = generateRandomPort();
            } else if (rv != 0) {
                std::string errorString = "Create ";
                errorString.append(endpointType).append("listener at ").append(url).append(":").append(
                        std::to_string(lowestPort));
                throw NngError(rv, errorString);
            }
        }
        try {
            componentManifest.addListenPort(endpointType, lowestPort);
            resourceDiscoveryConnEndpoint.addListenerPortForAllHubs(endpointType, lowestPort);
        } catch (AccessError &e) {
            //IGNORED AS THIS JUST MEANS WE HAVE A PAIR
        }
        std::cout << "Created endpoint of type: " << endpointType << "\n\tListening on URL: " << url << ":"
                  << lowestPort;
        std::cout << "\n\tLocal ID of socket: " << socketId <<  "\n\tSocket Type: " ;
        std::cout << componentManifest.getSocketType(endpointType) << std::endl;
        return lowestPort++;
    }else{
        return e->getListenPort();
    }
}

void Component::createEndpointAndDial(const std::string &localEndpointType, const std::string &dialUrl) {
    std::shared_ptr<DataReceiverEndpoint> e;
    std::string socketId = generateNewSocketId();
    createNewEndpoint(localEndpointType, socketId);
    try {
        e = getReceiverEndpointById(socketId);
    } catch (std::out_of_range &e) {
        throw std::logic_error("Couldn't make endpoint of type " + localEndpointType);
    }

    this->lowestPort++;
    try {
        e->dialConnection(dialUrl.c_str());
        std::cout << "Created endpoint of type: " << localEndpointType << "\n\tDial on URL: " << dialUrl;
        std::cout << "\n\tLocal ID of socket: " << socketId << "\n\tSocket Type: " ;
        std::cout << componentManifest.getSocketType(localEndpointType) << std::endl;
    } catch (NngError &e) {
        closeAndInvalidateSocketById(socketId);
        throw;
    }
}


void Component::startResourceDiscoveryHub(int port) {
    if (resourceDiscoveryHubEndpoint == nullptr) {
        this->resourceDiscoveryHubEndpoint = new ResourceDiscoveryHubEndpoint(systemSchemas);
        std::string listenAddress;
        if (componentConnectionType == ConnectionType::TCP) {
            listenAddress = "tcp://*";
        } else {
            listenAddress = selfAddress;
        }
        resourceDiscoveryHubEndpoint->startResourceDiscover(listenAddress, port);
        std::cout << "Started Resource Discovery Hub at - " << listenAddress << ":" << port << std::endl;
        resourceDiscoveryConnEndpoint.registerWithHub(selfAddress + ":" + std::to_string(port));
    }
}

int Component::startResourceDiscoveryHub() {
    if (resourceDiscoveryHubEndpoint == nullptr) {
        int nextPort = DEFAULT_RESOURCE_DISCOVERY_PORT;
        for (int i = 0; i < 5; i++) {
            try {
                startResourceDiscoveryHub(nextPort);
                return nextPort;
            } catch (NngError &e) {
                if (e.errorCode == NNG_EADDRINUSE) {
                    this->lowestPort = generateRandomPort();
                    nextPort = this->lowestPort;
                } else {
                    throw;
                }
            }
        }
        throw std::logic_error("Could not find valid port to start on");
    }
    return getResourceDiscoveryHubPort();
}

void Component::closeResourceDiscoveryHub() {
    if (resourceDiscoveryHubEndpoint != nullptr) {
        delete resourceDiscoveryHubEndpoint;
        resourceDiscoveryHubEndpoint = nullptr;
        std::cout << "Resource Discovery Hub ended" << std::endl;
    }
}


Component::~Component() {
    delete resourceDiscoveryHubEndpoint;
}

void Component::closeAndInvalidateSocketsOfType(const std::string &endpointType) {
    if (typeReceiverEndpoints.count(endpointType) > 0) {
        auto vec = typeReceiverEndpoints.at(endpointType);
        auto it = vec->begin();
        while (it != vec->end()) {
            (*it)->closeEndpoint();
            (*it)->invalidateEndpoint();
            it = vec->erase(it);
            idReceiverEndpoints.erase((*it)->getEndpointId());
        }
    }
    if (typeSenderEndpoints.count(endpointType) > 0) {
        auto vec = typeSenderEndpoints.at(endpointType);
        auto it = vec->begin();
        while (it != vec->end()) {
            (*it)->closeEndpoint();
            (*it)->invalidateEndpoint();
            it = vec->erase(it);
            idSenderEndpoints.erase((*it)->getEndpointId());
        }
    }
    if(componentManifest.hasListenPort(endpointType)){
        componentManifest.removeListenPort(endpointType);
    }
}


int Component::getResourceDiscoveryHubPort() {
    if (resourceDiscoveryHubEndpoint != nullptr) {
        return resourceDiscoveryHubEndpoint->getBackgroundPort();
    } else {
        throw std::logic_error("No resource discovery hub is opne");
    }
}

void Component::closeAndInvalidateSocketById(const std::string &endpointId) {
    // We do the same thing for receiver and senders.
    // Endpoints which are both don't matter about being closed twice


    auto receiverEndpoint = idReceiverEndpoints.find(endpointId);
    if (receiverEndpoint != idReceiverEndpoints.end()) {
        // Close the socket itself
        receiverEndpoint->second->closeEndpoint();
        receiverEndpoint->second->invalidateEndpoint();

        // Get our endpoint out of the "By Type" container
        auto possibleEndpointContainer = typeReceiverEndpoints.find(
                receiverEndpoint->second->getEndpointType());
        if (possibleEndpointContainer != typeReceiverEndpoints.end()) {
            std::vector<std::shared_ptr<DataReceiverEndpoint>>::iterator it;
            it = possibleEndpointContainer->second->begin();
            // Iterate over our possible container, used iterator due to deletions
            while (it != possibleEndpointContainer->second->end()) {
                // If the pointers are equal (so they point at the same thing)
                if ((*it) == receiverEndpoint->second) {
                    it = possibleEndpointContainer->second->erase(it);
                } else {
                    it++;
                }
            }
        }
        if(componentManifest.hasListenPort(receiverEndpoint->second->getEndpointType())){
            componentManifest.removeListenPort(receiverEndpoint->second->getEndpointType());
        }
        idReceiverEndpoints.erase(receiverEndpoint);
    }

    auto senderEndpoint = idSenderEndpoints.find(endpointId);
    if (senderEndpoint != idSenderEndpoints.end()) {
        senderEndpoint->second->closeEndpoint();
        senderEndpoint->second->invalidateEndpoint();

        auto possibleEndpointContainer = typeSenderEndpoints.find(senderEndpoint->second->getEndpointType());
        if (possibleEndpointContainer != typeSenderEndpoints.end()) {
            std::vector<std::shared_ptr<DataSenderEndpoint>>::iterator it;
            it = possibleEndpointContainer->second->begin();
            while (it != possibleEndpointContainer->second->end()) {
                if ((*it) == senderEndpoint->second) {
                    it = possibleEndpointContainer->second->erase(it);
                } else {
                    it++;
                }
            }
        }

        idSenderEndpoints.erase(senderEndpoint);
    }
}

void Component::closeAndInvalidateAllSockets() {
    for (const auto &endpointType : componentManifest.getAllEndpointTypes()) {
        closeAndInvalidateSocketsOfType(endpointType);
    }
}

void
Component::registerStartupFunction(const std::string &endpointType, endpointStartupFunction startupFunction, void *startupData) {
    if(componentManifest.hasEndpoint(endpointType)) {
        startupFunctionsMap[endpointType] = startupFunction;
        if(startupData != nullptr){
            startupDataMap[endpointType] = startupData;
        }
    }
}



std::shared_ptr<PairEndpoint> Component::castToPair(std::shared_ptr<DataReceiverEndpoint> e) {
    if(componentManifest.getSocketType(e->getEndpointType()) == PAIR && e->getEndpointSocketType() == SocketType::Pair){
        return std::static_pointer_cast<PairEndpoint>(e);
    }else{
        throw AccessError("Endpoint not a pair");
    }
}

std::shared_ptr<PairEndpoint> Component::castToPair(std::shared_ptr<DataSenderEndpoint> e) {
    if(componentManifest.getSocketType(e->getEndpointType()) == PAIR && e->getEndpointSocketType() == SocketType::Pair){
        return std::static_pointer_cast<PairEndpoint>(e);
    }else{
        throw AccessError("Endpoint not a pair");
    }
}

PairEndpoint *Component::castToPair(Endpoint *e) {
    if(componentManifest.getSocketType(e->getEndpointType()) == PAIR && e->getEndpointSocketType() == SocketType::Pair){
        return dynamic_cast<PairEndpoint*>(e);
    }else{
        throw AccessError("Endpoint not a pair");
    }
}
