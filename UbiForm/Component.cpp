
#include "Component.h"
#include "Utilities/SystemEnums.h"

#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
#include "Utilities/GetIPAddress.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>




// CONSTRUCTOR
Component::Component(const std::string &baseAddress) :  systemSchemas(),
        backgroundListener(this,systemSchemas), backgroundRequester(this, systemSchemas),
        baseAddress(baseAddress), resourceDiscoveryConnEndpoint(this, systemSchemas), componentManifest(systemSchemas){
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);
    std::cout << "Component made, base address is " << baseAddress << std::endl;
    if(baseAddress.rfind("tcp",0)==0){
        componentConnectionType = ConnectionType::LocalTCP;
        availableAddresses.push_back(baseAddress);
    }else if (baseAddress.rfind("ipc", 0) ==0){
        componentConnectionType = ConnectionType::IPC;
        availableAddresses.push_back(baseAddress);
    }else{
        throw std::logic_error("The given address did not have a type of TCP or IPC");
    }
}

Component::Component():  systemSchemas(),
        backgroundListener(this,systemSchemas), backgroundRequester(this, systemSchemas),
         resourceDiscoveryConnEndpoint(this, systemSchemas), componentManifest(systemSchemas) {
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);

    auto addresses = getLinuxIpAddresses();
    for (const auto & address: addresses){
        if(address.rfind("127.",0) != 0){
            availableAddresses.emplace_back(address);
        }
    }
    if (availableAddresses.empty()){
        throw std::logic_error("Could not find any networks to act on");
    }
    baseAddress = "tcp://127.0.0.1";
    componentConnectionType = ConnectionType::TCP;
    std::cout << "Component made, base address is " << baseAddress << std::endl;
}



// CREATE ENDPOINTS (don't connect)
std::shared_ptr<PairEndpoint> Component::createNewPairEndpoint(const std::string& typeOfEndpoint, const std::string& id){
    std::shared_ptr<EndpointSchema>recvSchema = componentManifest.getReceiverSchema(typeOfEndpoint);
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest.getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PairEndpoint> pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema, id);
    idReceiverEndpoints.insert(std::make_pair(id, pe));
    idSenderEndpoints.insert(std::make_pair(id, pe));


    if (typeSenderEndpoints.count(typeOfEndpoint) == 1) {
        typeSenderEndpoints.at(typeOfEndpoint)->push_back(pe);
    }else{
        typeSenderEndpoints.insert(std::make_pair(typeOfEndpoint,std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >(1,pe)));
    }

    if (typeReceiverEndpoints.count(typeOfEndpoint) == 1) {
        typeReceiverEndpoints.at(typeOfEndpoint)->push_back(pe);
    }else{
        typeReceiverEndpoints.insert(std::make_pair(typeOfEndpoint,std::make_shared<std::vector<std::shared_ptr<DataReceiverEndpoint> > >(1,pe)));
    }

    return pe;
}

std::shared_ptr<PublisherEndpoint> Component::createNewPublisherEndpoint(const std::string& typeOfEndpoint, const std::string& id) {
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest.getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PublisherEndpoint> pe = std::make_shared<PublisherEndpoint>(sendSchema, id);
    idSenderEndpoints.insert(std::make_pair(id, pe));

    if (typeSenderEndpoints.count(typeOfEndpoint) == 1) {
        typeSenderEndpoints.at(typeOfEndpoint)->push_back(pe);
    }else{
        typeSenderEndpoints.insert(std::make_pair(typeOfEndpoint,std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >(1,pe)));
    }

    return pe;
}

std::shared_ptr<SubscriberEndpoint> Component::createNewSubscriberEndpoint(const std::string& typeOfEndpoint, const std::string& id) {
    std::shared_ptr<EndpointSchema>receiveSchema = componentManifest.getReceiverSchema(typeOfEndpoint);

    std::shared_ptr<SubscriberEndpoint> pe = std::make_shared<SubscriberEndpoint>(receiveSchema, id);
    idReceiverEndpoints.insert(std::make_pair(id, pe));

    if (typeReceiverEndpoints.count(typeOfEndpoint) == 1) {
        typeReceiverEndpoints.at(typeOfEndpoint)->push_back(pe);
    }else{
        typeReceiverEndpoints.insert(std::make_pair(typeOfEndpoint,std::make_shared<std::vector<std::shared_ptr<DataReceiverEndpoint> > >(1,pe)));
    }

    return pe;
}


// GET OUR ENDPOINTS BY ID
std::shared_ptr<DataReceiverEndpoint> Component::getReceiverEndpointById(const std::string &id) {
    try{
        return idReceiverEndpoints.at(id);
    }catch(std::out_of_range &e){
        throw;
    }
}

std::shared_ptr<DataSenderEndpoint> Component::getSenderEndpointById(const std::string &id) {
    try{
        return idSenderEndpoints.at(id);
    }catch(std::out_of_range &e){
        throw;
    }
}

// GET OUR ENDPOINTS BY ENDPOINTTYPE
std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > >
Component::getReceiverEndpointsByType(const std::string &endpointType) {
    try {
        return typeReceiverEndpoints.at(endpointType);
    }catch(std::out_of_range &e){
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
    }catch(std::out_of_range &e){
        // Make an empty vector to return, this will get filled if things then come along later
        auto returnVector = std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >();
        typeSenderEndpoints.insert(std::make_pair(endpointType, returnVector));
        return returnVector;
    }
}

// THIS IS OUR COMPONENT LISTENING FOR REQUESTS TO MAKE SOCKETS
void Component::startBackgroundListen() {
    for(int i = 0; i < 5 ; i++) {
        try {
            startBackgroundListen(this->lowestPort);
            this->lowestPort++;
            return;
        } catch (NngError &e) {
            if(e.errorCode == NNG_EADDRINUSE){
                this->lowestPort = generateRandomPort();
            }else{
                throw;
            }
        }
    }
    throw std::logic_error("Could not find valid port to start on");
}
void Component::startBackgroundListen(int port) {
    if(componentConnectionType == ConnectionType::TCP) {
        // Listen on all addresses
        backgroundListener.startBackgroundListen("tcp://*" , port);
    }else{
        // Listen on just the address given (either local or IPC)
        backgroundListener.startBackgroundListen(baseAddress, port);
    }
}

int Component::createEndpointAndListen(SocketType st, const std::string& endpointType){
    std::string socketId = generateNewSocketId();
    std::shared_ptr<DataSenderEndpoint> e;
    switch (st) {
        case Pair:
            e = createNewPairEndpoint(endpointType, socketId);
            break;
        case Publisher:
            e = createNewPublisherEndpoint(endpointType, socketId);
            break;
        default:
            throw std::logic_error("Cannot open a connection for type " + std::to_string(st));
    }


    int rv = 1;
    std::string url;
    while(rv != 0) {
        if(componentConnectionType == ConnectionType::TCP){
            url = "tcp://*";
        }else {
            url = baseAddress;
        }
        rv = e->listenForConnectionWithRV(url.c_str(), lowestPort);
        if (rv == EADDRINUSE){
            lowestPort = generateRandomPort();
        }else if (rv != 0){
            throw NngError(rv, "Create " + convertSocketType(st) + " listener at " + url);
        }
    }
    std::cout << "Created endpoint of type: " << endpointType << "\n\tListening on URL: " << url << ":" << lowestPort << std::endl;
    return lowestPort++;
}

void Component::createEndpointAndDial(const std::string& socketType, const std::string& localEndpointType, const std::string& url){
    std::shared_ptr<DataReceiverEndpoint> e;
    std::string socketId = generateNewSocketId();
    if (socketType == PAIR){
        e = this->createNewPairEndpoint(localEndpointType, socketId);
    }else if (socketType == SUBSCRIBER){
        e = this->createNewSubscriberEndpoint(localEndpointType, socketId);
    }

    this->lowestPort ++;
    e->dialConnection(url.c_str());
    std::cout << "Created endpoint of type: " << localEndpointType << "\n\tDial on URL: " << url << std::endl;

}


// CREATE RDHUB
void Component::startResourceDiscoveryHub(int port) {
    if (resourceDiscoveryHubEndpoint == nullptr) {
        this->resourceDiscoveryHubEndpoint = new ResourceDiscoveryHubEndpoint(systemSchemas);
        std::string listenAddress;
        if (componentConnectionType == ConnectionType::TCP) {
            listenAddress = "tcp://*";
        }else{
            listenAddress = baseAddress;
        }
        resourceDiscoveryHubEndpoint->startResourceDiscover(listenAddress, port);
        std::cout << "Started Resource Discovery Hub at - " << listenAddress << std::endl;
    }
}
int Component::startResourceDiscoveryHub() {
    for(int i = 0; i < 5 ; i++) {
        try {
            startResourceDiscoveryHub(this->lowestPort);
            return this->lowestPort++;
        } catch (NngError &e) {
            if(e.errorCode == NNG_EADDRINUSE){
                this->lowestPort = generateRandomPort();
            }else{
                throw;
            }
        }
    }
    throw std::logic_error("Could not find valid port to start on");
}



Component::~Component(){
    delete resourceDiscoveryHubEndpoint;
}

void Component::closeSocketsOfType(const std::string &endpointType) {
    if(typeReceiverEndpoints.count(endpointType) > 0){
        auto vec = typeReceiverEndpoints.at(endpointType);
        auto it = vec->begin();
        while(it != vec->end()){
            (*it)->closeSocket();
            it = vec->erase(it);
        }
    }
    if(typeSenderEndpoints.count(endpointType) > 0){
        auto vec = typeSenderEndpoints.at(endpointType);
        auto it = vec->begin();
        while(it != vec->end()){
            (*it)->closeSocket();
            std::cout << "Remaining references: " << (*it).use_count() << std::endl;
            it = vec->erase(it);
        }
    }
}

std::string Component::getRDHLocation() {
    if(resourceDiscoveryHubEndpoint == nullptr){
        throw std::logic_error("No RDH has been made");
    }else{
        return resourceDiscoveryHubEndpoint->getListenAddress();
    }
}
