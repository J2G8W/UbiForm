
#include "Component.h"

#include "endpoints/PairEndpoint.h"
#include "endpoints/PublisherEndpoint.h"
#include "endpoints/SubscriberEndpoint.h"

#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>
#include <thread>


// NOTE THAT these strings are defined in the various schemas in SystemSchemas
#define PAIR "pair"
#define PUBLISHER "publisher"
#define SUBSCRIBER "subscriber"

// CONSTRUCTOR
Component::Component(const std::string &baseAddress) : backgroundSocket() {
    this->baseAddress = baseAddress;
    const char* files[3] = {"SystemSchemas/component_schema.json",
                            "SystemSchemas/endpoint_creation_request.json",
                            "SystemSchemas/endpoint_creation_response.json"};

    for (int i =0; i < 3; i++){
        FILE* pFile = fopen(files[i], "r");
        if (pFile == NULL){
            std::cerr << "Error finding requisite file - " << files[i] << std::endl;
            exit(1);
        }
        std::unique_ptr<EndpointSchema> es = std::make_unique<EndpointSchema>(pFile);
        systemSchemas.insert(std::make_pair(static_cast<ComponentSystemSchema>(i), std::move(es)));
        fclose(pFile);
    }
}



// CREATE ENDPOINTS (don't connect)
std::shared_ptr<PairEndpoint> Component::createNewPairEndpoint(std::string typeOfEndpoint, std::string id){
    std::shared_ptr<EndpointSchema>recvSchema = componentManifest->getReceiverSchema(typeOfEndpoint);
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PairEndpoint> pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema);
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

std::shared_ptr<PublisherEndpoint> Component::createNewPublisherEndpoint(std::string typeOfEndpoint, std::string id) {
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PublisherEndpoint> pe = std::make_shared<PublisherEndpoint>(sendSchema);
    idSenderEndpoints.insert(std::make_pair(id, pe));

    if (typeSenderEndpoints.count(typeOfEndpoint) == 1) {
        typeSenderEndpoints.at(typeOfEndpoint)->push_back(pe);
    }else{
        typeSenderEndpoints.insert(std::make_pair(typeOfEndpoint,std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >(1,pe)));
    }

    return pe;
}

std::shared_ptr<SubscriberEndpoint> Component::createNewSubscriberEndpoint(std::string typeOfEndpoint, std::string id) {
    std::shared_ptr<EndpointSchema>receiveSchema = componentManifest->getReceiverSchema(typeOfEndpoint);

    std::shared_ptr<SubscriberEndpoint> pe = std::make_shared<SubscriberEndpoint>(receiveSchema);
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
void Component::startBackgroundListen(int port) {
    int rv;
    if ((rv = nng_rep0_open(&backgroundSocket)) != 0) {
        fatal("Failure opening background socket", rv);
    }
    this->backgroundListenAddress = this->baseAddress + ":" + std::to_string(port);
    if ((rv = nng_listen(backgroundSocket, backgroundListenAddress.c_str(), nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    this->lowestPort ++;

    this->backgroundThread = std::thread(backgroundListen,this);

}


void Component::backgroundListen(Component *component) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(component->backgroundSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }
        SocketMessage sm(buf);
        nng_free(buf, sz);
        try {
            component->systemSchemas.at(ComponentSystemSchema::endpointCreationRequest)->validate(sm);
            if (sm.getString("socketType") == PAIR) {
                std::string url = component->baseAddress + ":" + std::to_string(component->lowestPort);

                std::shared_ptr<DataSenderEndpoint> e = component->createNewPairEndpoint(sm.getString("endpointType"), std::to_string(
                        component->lowestPort));
                e->listenForConnection(url.c_str());

                component->lowestPort++;

                SocketMessage reply;
                reply.addMember("url",url);
                component->systemSchemas.at(ComponentSystemSchema::endpointCreationResponse)->validate(reply);
                std::string replyText = reply.stringify();

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
                    std::ostringstream error;
                    error << "Error replying" << std::endl;
                    error << "NNG error code " << rv << " type - " << nng_strerror(rv);
                    throw std::logic_error(error.str());
                }
            } else if (sm.getString("socketType") == PUBLISHER) {
                std::string url;
                auto existingPublishers = component->getSenderEndpointsByType(sm.getString("endpointType"));
                if (existingPublishers->empty()) {
                    url = component->baseAddress + ":"+ std::to_string(component->lowestPort);
                    std::shared_ptr<DataSenderEndpoint> e =
                            component->createNewPublisherEndpoint(sm.getString("endpointType"), std::to_string(component->lowestPort));
                    e->listenForConnection(url.c_str());
                    component->lowestPort++;
                }else{
                    url = existingPublishers->at(0)->getListenUrl();
                }

                SocketMessage reply;
                reply.addMember("url",url);
                component->systemSchemas.at(ComponentSystemSchema::endpointCreationResponse)->validate(reply);
                std::string replyText = reply.stringify();

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
                    throw NNG_error(rv, "Regrep reply");
                }
            }
        }catch (std::out_of_range &e){
            std::cerr << "No schema of type " << sm.getString("endpointType") << " found in this component." <<  std::endl << e.what() <<std::endl;
            SocketMessage reply;
            reply.setNull("url");
            component->systemSchemas.at(ComponentSystemSchema::endpointCreationResponse)->validate(reply);
            std::string replyText = reply.stringify();
            if ((rv = nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
               fatal("nng_send", rv);
            }
        }catch (std::logic_error &e){
            std::cerr << e.what() << std::endl;
        }
    }
}

// THIS IS OUR COMPONENT MAKING REQUESTS FOR A NEW CONNECTION
void Component::requestAndCreateConnection(const std::string& localEndpointType, const std::string &connectionComponentAddress,
                                           const std::string &remoteEndpointType) {
    std::string requestSocketType = componentManifest->getSocketType(localEndpointType);
    SocketMessage sm;
    if (requestSocketType == SUBSCRIBER){
        sm.addMember("socketType",PUBLISHER);
    }else{
        sm.addMember("socketType",requestSocketType);
    }

    sm.addMember("endpointType",remoteEndpointType);

    systemSchemas.at(ComponentSystemSchema::endpointCreationRequest)->validate(sm);

    std::string url;

    try{
        url = requestConnection(connectionComponentAddress,sm.stringify());
        std::shared_ptr<DataReceiverEndpoint> e;
        if (requestSocketType == PAIR){
            e = this->createNewPairEndpoint(localEndpointType, std::to_string(this->lowestPort));
        }else if (requestSocketType == SUBSCRIBER){
            e = this->createNewSubscriberEndpoint(localEndpointType, std::to_string(this->lowestPort));
        }

        this->lowestPort ++;
        e->dialConnection(url.c_str());
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

std::string Component::requestConnection(const std::string &address, const std::string& requestText) {
    int rv;

    nng_socket tempSocket;
    if ((rv = nng_req0_open(&tempSocket)) != 0) {
        fatal("Failure opening temporary socket", rv);
    }

    if ((rv = nng_dial(tempSocket, address.c_str(), nullptr, 0)) != 0) {
        fatal("nng_dial", rv);
    }

    if ((rv = nng_send(tempSocket, (void *) requestText.c_str(), requestText.size() + 1, 0)) != 0) {
        fatal("nng_send", rv);
    }

    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(tempSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        fatal("nng_recv", rv);
    }
    try{
        SocketMessage response(buf);
        nng_free(buf,sz);


        if (response.isNull("url")){
            throw std::logic_error("No valid endpoint of: " + requestText);
        }else{
            try {
                return response.getString("url");
            }catch (AccessError &e){
                throw std::logic_error("No endpoint returned");
            }
        }
    } catch (std::logic_error &e) {
        std::cerr << "Error in handling response" << std::endl;
        throw;
    }

}

// CREATE RDCONNECTION
ResourceDiscoveryConnEndpoint *Component::createResourceDiscoveryConnectionEndpoint() {
    if(this->resourceDiscoveryConnEndpoint == nullptr) {
        this->resourceDiscoveryConnEndpoint = new ResourceDiscoveryConnEndpoint(this);
    }
    return this->resourceDiscoveryConnEndpoint;
}

// CREATE RDHUB
void Component::startResourceDiscoveryHub(int port) {
    auto* rdh = new ResourceDiscoveryHubEndpoint;
    std::string listenAddress = baseAddress + ":" + std::to_string(port);
    rdh->startResourceDiscover(listenAddress);
}


Component::~Component(){
    // We detach our background thread so termination of the thread happens safely
    if (backgroundThread.joinable()) {
        backgroundThread.detach();
    }

    // Make sure that the messages are flushed
    nng_msleep(300);


    // Close our background socket, and don't really care what return value is
    // TODO - sort problem of closing socket while backgroundThread uses it
    //nng_close(backgroundSocket);
}
