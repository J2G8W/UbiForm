
#include "Component.h"
#include "SystemEnums.h"

#include "ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>




// CONSTRUCTOR
Component::Component(const std::string &baseAddress) : backgroundSocket(), systemSchemas() {
    this->baseAddress = baseAddress;
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);
    // Create the background socket;
    int rv;
    if ((rv = nng_rep0_open(&backgroundSocket)) != 0) {
        throw NngError(rv, "Opening background socket");
    }
}



// CREATE ENDPOINTS (don't connect)
std::shared_ptr<PairEndpoint> Component::createNewPairEndpoint(const std::string& typeOfEndpoint, const std::string& id){
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

std::shared_ptr<PublisherEndpoint> Component::createNewPublisherEndpoint(const std::string& typeOfEndpoint, const std::string& id) {
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

std::shared_ptr<SubscriberEndpoint> Component::createNewSubscriberEndpoint(const std::string& typeOfEndpoint, const std::string& id) {
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
    this->backgroundListenAddress = this->baseAddress + ":" + std::to_string(port);
    if ((rv = nng_listen(backgroundSocket, backgroundListenAddress.c_str(), nullptr, 0)) != 0) {
        throw NngError(rv, "Listening on " + backgroundListenAddress);
    }

    this->backgroundThread = std::thread(backgroundListen,this);

}

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


void Component::backgroundListen(Component *component) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(component->backgroundSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            std::cerr << "NNG error receiving component request - " << nng_strerror(rv) << std::endl;
            continue;
        }
        SocketMessage sm(buf);
        nng_free(buf, sz);
        try {
            component->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);
            if (sm.getString("socketType") == PAIR) {
                std::string socketId = component->generateNewSocketId();

                std::shared_ptr<DataSenderEndpoint> e = component->createNewPairEndpoint(
                        sm.getString("endpointType"), socketId);

                std::string url;
                rv = 1;
                while(rv != 0) {
                    url = component->baseAddress + ":" + std::to_string(component->lowestPort);
                    rv = e->listenForConnectionWithRV(url.c_str());
                    if (rv == EADDRINUSE){
                        component->lowestPort = component->generateRandomPort();
                    }else if (rv != 0){
                        throw NngError(rv, "Create pair listener at " + url);
                    }
                }

                component->lowestPort++;

                SocketMessage reply;
                reply.addMember("url",url);
                component->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(reply);
                std::string replyText = reply.stringify();

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
                    throw NngError(rv, "Replying to Pair creation request");
                }
            } else if (sm.getString("socketType") == PUBLISHER) {
                std::string url;
                auto existingPublishers = component->getSenderEndpointsByType(sm.getString("endpointType"));
                if (existingPublishers->empty()) {
                    std::string socketId = component->generateNewSocketId();

                    std::shared_ptr<DataSenderEndpoint> e = component->createNewPublisherEndpoint(
                            sm.getString("endpointType"), socketId);

                    rv = 1;
                    while(rv != 0) {
                        url = component->baseAddress + ":" + std::to_string(component->lowestPort);
                        rv = e->listenForConnectionWithRV(url.c_str());
                        if (rv == EADDRINUSE){
                            component->lowestPort = component->generateRandomPort();
                        }else if (rv != 0){
                            throw NngError(rv, "Create pair listener at " + url);
                        }
                    }
                    component->lowestPort ++;
                }else{
                    url = existingPublishers->at(0)->getListenUrl();
                }

                SocketMessage reply;
                reply.addMember("url",url);
                component->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(reply);
                std::string replyText = reply.stringify();

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
                    throw NngError(rv, "Replying to Publisher creation request");
                }
            }
        }catch (std::out_of_range &e){
            std::cerr << "No schema of type " << sm.getString("endpointType") << " found in this component." <<  std::endl;
            // Return a simple reply, any errors on send are IGNORED
            SocketMessage reply;
            reply.setNull("url");
            component->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(reply);
            std::string replyText = reply.stringify();
            if (nng_send(component->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0) != 0) {
               // IGNORE
            }
        }catch(ValidationError &e){
            std::cerr << "Invalid creation request - " << e.what() <<std::endl;
        }catch(NngError &e){
            std::cerr << "NNG error in handling creation request - " << e.what() <<std::endl;
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

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);

    std::string url;

    try{
        url = requestConnection(connectionComponentAddress,sm.stringify());
        std::shared_ptr<DataReceiverEndpoint> e;
        std::string socketId = generateNewSocketId();
        if (requestSocketType == PAIR){
            e = this->createNewPairEndpoint(localEndpointType, socketId);
        }else if (requestSocketType == SUBSCRIBER){
            e = this->createNewSubscriberEndpoint(localEndpointType, socketId);
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
        throw NngError(rv, "Opening temporary socket for connection request");
    }

    if ((rv = nng_dial(tempSocket, address.c_str(), nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing " + address + " for connection request");
    }

    if ((rv = nng_send(tempSocket, (void *) requestText.c_str(), requestText.size() + 1, 0)) != 0) {
        throw NngError(rv, "Sending to " + address + " for connection request");
    }

    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(tempSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        throw NngError(rv, "Receiving response from connection request");
    }
    try{
        SocketMessage response(buf);
        nng_free(buf,sz);

        if (response.isNull("url")){
            throw std::logic_error("No valid endpoint of: " + requestText);
        }else{
            try {
                // Basically validation of response (CBA for using schemas for one field)
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
        this->resourceDiscoveryConnEndpoint = new ResourceDiscoveryConnEndpoint(this, systemSchemas);
    }
    return this->resourceDiscoveryConnEndpoint;
}

// CREATE RDHUB
void Component::startResourceDiscoveryHub(int port) {
    auto* rdh = new ResourceDiscoveryHubEndpoint(systemSchemas);
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
