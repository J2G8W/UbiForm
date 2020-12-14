
#include "Component.h"

#include "endpoints/PairEndpoint.h"
#include "endpoints/PublisherEndpoint.h"
#include "endpoints/SubscriberEndpoint.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>
#include <thread>

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

std::shared_ptr<std::vector<std::shared_ptr<DataReceiverEndpoint> > >
Component::getReceiverEndpointsByType(const std::string &type) {
    try {
        return typeReceiverEndpoints.at(type);
    }catch(std::out_of_range &e){
        // Make an empty vector to return, this will get filled if things then come along later
        auto returnVector = std::make_shared<std::vector<std::shared_ptr<DataReceiverEndpoint> > >();
        typeReceiverEndpoints.insert(std::make_pair(type, returnVector));
        return returnVector;
    }
}


std::shared_ptr<std::vector<std::shared_ptr<DataSenderEndpoint> > >
Component::getSenderEndpointsByType(const std::string &type) {
    try {
        return typeSenderEndpoints.at(type);
    }catch(std::out_of_range &e){
        // Make an empty vector to return, this will get filled if things then come along later
        auto returnVector = std::make_shared<std::vector<std::shared_ptr<DataSenderEndpoint> > >();
        typeSenderEndpoints.insert(std::make_pair(type, returnVector));
        return returnVector;
    }
}


void Component::startBackgroundListen(const char * listenAddress) {
    int rv;
    if ((rv = nng_rep0_open(&backgroundSocket)) != 0) {
        fatal("Failure opening background socket", rv);
    }

    if ((rv = nng_listen(backgroundSocket, listenAddress, nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    this->lowestPort ++;
    this->backgroundThread = std::thread(backgroundListen,this);

}

#define PAIR "PAIR"
#define ERROR "ERROR"
#define PUBLISHER "PUB"

void Component::backgroundListen(Component *component) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(component->backgroundSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }

        try {
            if ((sz > strlen(PAIR)) && strncmp(buf,PAIR,strlen(PAIR)) == 0) {
                char *typeRequest = buf + strlen(PAIR);
                std::string url = "tcp://127.0.0.1:" + std::to_string(component->lowestPort);

                std::shared_ptr<DataSenderEndpoint> e = component->createNewPairEndpoint(typeRequest, std::to_string(
                        component->lowestPort));
                e->listenForConnection(url.c_str());

                component->lowestPort++;

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) url.c_str(), url.size() + 1, 0)) != 0) {
                    std::ostringstream error;
                    error << "Error replying" << std::endl;
                    error << "NNG error code " << rv << " type - " << nng_strerror(rv);
                    throw std::logic_error(error.str());
                }
            } else if ((sz > strlen(PUBLISHER)) && strncmp(buf,PUBLISHER,strlen(PUBLISHER)) == 0) {
                char *typeRequest = buf + strlen(PUBLISHER);

                std::string url;
                // TODO - change this to be a standard boolean check
                auto existingPublishers = component->getSenderEndpointsByType(typeRequest);
                if (existingPublishers->empty()) {
                    url = "tcp://127.0.0.1:" + std::to_string(component->lowestPort);
                    std::shared_ptr<DataSenderEndpoint> e =
                            component->createNewPublisherEndpoint(typeRequest, std::to_string(component->lowestPort));
                    e->listenForConnection(url.c_str());
                    component->lowestPort++;
                }else{
                    url = existingPublishers->at(0)->getListenUrl();
                }


                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) url.c_str(), url.size() + 1, 0)) != 0) {
                    throw NNG_error(rv, "Regrep reply");
                }
            }
        }catch (std::out_of_range &e){
            std::cerr << "No schema of type " << buf << " found in this component." <<  std::endl << e.what() <<std::endl;
            const char *reply = ERROR;
            if ((rv = nng_send(component->backgroundSocket, (void *) reply, strlen(reply) + 1, 0)) != 0) {
               fatal("nng_send", rv);
            }
        }catch (std::logic_error &e){
            std::cerr << e.what() << std::endl;
        }
    }
}

void Component::requestPairConnection(const std::string& address, const std::string& endpointType){

    std::string request = PAIR + endpointType;
    char* url;
    try{
        url = requestConnection(address,request);
        std::shared_ptr<DataReceiverEndpoint> e = this->createNewPairEndpoint(endpointType, std::to_string(this->lowestPort));
        this->lowestPort ++;
        e->dialConnection(url);
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }

    delete url;
}

void Component::requestConnectionToPublisher(const std::string &address, const std::string &endpointType) {
    std::string request = PUBLISHER + endpointType;
    char* url;
    try{
        url = requestConnection(address,request);
        std::shared_ptr<DataReceiverEndpoint> e = this->createNewSubscriberEndpoint(endpointType, std::to_string(this->lowestPort));
        this->lowestPort ++;
        e->dialConnection(url);
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }

    delete url;
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

char *Component::requestConnection(const std::string &address, const std::string& requestText) {
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
    if ((sz > strlen(ERROR)) && strncmp(buf,ERROR,strlen(ERROR)) == 0){
        throw std::logic_error("No valid endpoint of: " + requestText);
    }else {
        buf[sz - 1] = 0;
        return buf;
    }
}
