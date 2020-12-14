
#include "Component.h"

#include "endpoints/PairEndpoint.h"
#include "endpoints/PublisherEndpoint.h"
#include "endpoints/SubscriberEndpoint.h"

#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <thread>

std::shared_ptr<PairEndpoint> Component::createNewPairEndpoint(std::string typeOfEndpoint, std::string id){
    std::shared_ptr<EndpointSchema>recvSchema = componentManifest->getReceiverSchema(typeOfEndpoint);
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PairEndpoint> pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema);
    receiverEndpoints.insert(std::make_pair(id, pe));
    senderEndpoints.insert(std::make_pair(id, pe));
    return pe;
}

void Component::createNewPublisherEndpoint(std::string typeOfEndpoint, std::string id) {
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PublisherEndpoint> pe = std::make_shared<PublisherEndpoint>(sendSchema);
    senderEndpoints.insert(std::make_pair(id, pe));
}

void Component::createNewSubscriberEndpoint(std::string typeOfEndpoint, std::string id) {
    std::shared_ptr<EndpointSchema>receiveSchema = componentManifest->getReceiverSchema(typeOfEndpoint);

    std::shared_ptr<SubscriberEndpoint> pe = std::make_shared<SubscriberEndpoint>(receiveSchema);
    receiverEndpoints.insert(std::make_pair(id, pe));
}

std::shared_ptr<DataReceiverEndpoint> Component::getReceiverEndpoint(const std::string &id) {
    try{
        return receiverEndpoints.at(id);
    }catch(std::out_of_range &e){
        throw;
    }
}

std::shared_ptr<DataSenderEndpoint> Component::getSenderEndpoint(const std::string &id) {
    try{
        return senderEndpoints.at(id);
    }catch(std::out_of_range &e){
        throw;
    }
}

void Component::startBackgroundListen() {
    int rv;
    if ((rv = nng_rep0_open(&backgroundSocket)) != 0) {
        fatal("Failure opening background socket", rv);
    }

    if ((rv = nng_listen(backgroundSocket, "tcp://127.0.0.1:8000", nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    this->lowestPort ++;
    this->backgroundThread = std::thread(backgroundListen,this);

}

#define PAIR "PAIR"
#define ERROR "ERROR"
void Component::backgroundListen(Component *component) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(component->backgroundSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }
        if ((sz > strlen(PAIR)) && strncmp(buf,PAIR,strlen(PAIR)) == 0){
            char * typeRequest = buf + strlen(PAIR);
            try {
                component->componentManifest->getSenderSchema(typeRequest);

                std::string url = "tcp://127.0.0.1:" + std::to_string(component->lowestPort);

                std::shared_ptr<DataSenderEndpoint> e = component->createNewPairEndpoint(typeRequest, std::to_string(component->lowestPort));
                e->listenForConnection(url.c_str());

                component->lowestPort ++;

                // Send reply on regrep with url for the component to dial
                if ((rv = nng_send(component->backgroundSocket, (void *) url.c_str(), url.size() + 1, 0)) != 0) {
                    std::ostringstream error;
                    error << "Error replying" << std::endl;
                    error << "NNG error code " << rv << " type - " << nng_strerror(rv);
                    throw std::logic_error(error.str());
                }
            }catch (std::out_of_range &e){
                std::cerr << "No schema of type " << typeRequest << " found in this component" << std::endl;
                const char *reply = ERROR;
                if ((rv = nng_send(component->backgroundSocket, (void *) reply, strlen(reply) + 1, 0)) != 0) {
                    fatal("nng_send", rv);
                }
            }catch (std::logic_error &e){
                std::cerr << e.what() << std::endl;
            }
        }
    }
}

void Component::requestPairConnection(const std::string& address, const std::string& endpointType){
    int rv;
    nng_socket tempSocket;
    if ((rv = nng_req0_open(&tempSocket)) != 0) {
        fatal("Failure opening temporary socket", rv);
    }

    if ((rv = nng_dial(tempSocket, address.c_str(), nullptr, 0)) != 0) {
        fatal("nng_dial", rv);
    }
    std::string request = PAIR + endpointType;
    if ((rv = nng_send(tempSocket, (void *) request.c_str(), request.size() + 1, 0)) != 0) {
        fatal("nng_send", rv);
    }

    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(tempSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        fatal("nng_recv", rv);
    }
    if ((sz > strlen(ERROR)) && strncmp(buf,ERROR,strlen(ERROR)) == 0){
        std::cerr << "ERROR NO VALID ENDPOINT" << std::endl;
    }else{
        buf[sz-1] = 0;
        try{
            std::shared_ptr<DataReceiverEndpoint> e = this->createNewPairEndpoint(endpointType, std::to_string(this->lowestPort));
            this->lowestPort ++;
            e->dialConnection(buf);
        }catch (std::logic_error &e){
            std::cerr << e.what() << std::endl;
        }
    }
    delete buf;
}


Component::~Component(){
    // Make sure that the messages are flushed
    sleep(1);

    nng_close(backgroundSocket);
    // Background thread automatically terminated as it is in component.
}