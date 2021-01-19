
#include "Component.h"
#include "Utilities/SystemEnums.h"

#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
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
    closeAllSockets();
    componentManifest.setManifest(jsonFP);
    resourceDiscoveryConnEndpoint.updateManifestWithHubs();
}

void Component::specifyManifest(const char *jsonString) {
    closeAllSockets();
    componentManifest.setManifest(jsonString);
    resourceDiscoveryConnEndpoint.updateManifestWithHubs();
}

void Component::specifyManifest(SocketMessage *sm) {
    closeAllSockets();
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
    switch (socketType) {
        case Pair: {
            auto pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema, endpointType, endpointId);
            receiverEndpoint = pe;
            senderEndpoint = pe;
            break;
        }
        case Publisher:
            senderEndpoint = std::make_shared<PublisherEndpoint>(sendSchema, endpointType, endpointId);
            break;
        case Subscriber:
            receiverEndpoint = std::make_shared<SubscriberEndpoint>(recvSchema, endpointType, endpointId);
            break;
        case Reply: {
            auto pe = std::make_shared<ReplyEndpoint>(recvSchema, sendSchema, endpointType, endpointId);
            receiverEndpoint = pe;
            senderEndpoint = pe;
            break;
        }
        case Request: {
            auto pe = std::make_shared<RequestEndpoint>(recvSchema, sendSchema, endpointType, endpointId);
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
    createNewEndpoint(endpointType, socketId);
    try {
        e = getSenderEndpointById(socketId);
    } catch (std::out_of_range &e) {
        throw std::logic_error(
                "Couldn't make endpoint of type " + endpointType);
    }


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
            errorString.append(endpointType).append("listener at ").append(url).append(":").append(std::to_string(lowestPort));
            throw NngError(rv, errorString);
        }
    }
    std::cout << "Created endpoint of type: " << endpointType << "\n\tListening on URL: " << url << ":" << lowestPort;
    std::cout << "\n\tLocal ID of socket: " << socketId << std::endl;
    return lowestPort++;
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
        std::cout << "\n\tLocal ID of socket: " << socketId << std::endl;
    }catch(NngError &e){
        closeSocketOfId(socketId);
        throw;
    }
}


// CREATE RDHUB
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

void Component::closeSocketsOfType(const std::string &endpointType) {
    if (typeReceiverEndpoints.count(endpointType) > 0) {
        auto vec = typeReceiverEndpoints.at(endpointType);
        auto it = vec->begin();
        while (it != vec->end()) {
            (*it)->closeSocket();
            it = vec->erase(it);
            idReceiverEndpoints.erase((*it)->getReceiverEndpointID());
        }
    }
    if (typeSenderEndpoints.count(endpointType) > 0) {
        auto vec = typeSenderEndpoints.at(endpointType);
        auto it = vec->begin();
        while (it != vec->end()) {
            (*it)->closeSocket();
            it = vec->erase(it);
            idSenderEndpoints.erase((*it)->getSenderEndpointID());
        }
    }
}


int Component::getResourceDiscoveryHubPort() {
    if (resourceDiscoveryHubEndpoint != nullptr) {
        return resourceDiscoveryHubEndpoint->getBackgroundPort();
    } else {
        throw std::logic_error("No resource discovery hub is opne");
    }
}

void Component::closeSocketOfId(const std::string &endpointId) {
    // We do the same thing for receiver and senders.
    // Endpoints which are both don't matter about being closed twice


    auto receiverEndpoint = idReceiverEndpoints.find(endpointId);
    if (receiverEndpoint != idReceiverEndpoints.end()) {
        // Close the socket itself
        receiverEndpoint->second->closeSocket();

        // Get our endpoint out of the "By Type" container
        auto possibleEndpointContainer = typeReceiverEndpoints.find(
                receiverEndpoint->second->getReceiverEndpointType());
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

        idReceiverEndpoints.erase(receiverEndpoint);
    }

    auto senderEndpoint = idSenderEndpoints.find(endpointId);
    if (senderEndpoint != idSenderEndpoints.end()) {
        senderEndpoint->second->closeSocket();

        auto possibleEndpointContainer = typeSenderEndpoints.find(senderEndpoint->second->getSenderEndpointType());
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

void Component::closeAllSockets() {
    for(const auto& endpointType : componentManifest.getAllEndpointTypes()){
        closeSocketsOfType(endpointType);
    }
}

