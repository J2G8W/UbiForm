
#include "Component.h"

#include "endpoints/PairEndpoint.h"
#include "endpoints/PublisherEndpoint.h"
#include "endpoints/SubscriberEndpoint.h"

void Component::createNewPairEndpoint(std::string typeOfEndpoint, std::string id){
    std::shared_ptr<EndpointSchema>recvSchema = componentManifest->getReceiverSchema(typeOfEndpoint);
    std::shared_ptr<EndpointSchema>sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    std::shared_ptr<PairEndpoint> pe = std::make_shared<PairEndpoint>(recvSchema, sendSchema);
    receiverEndpoints.insert(std::make_pair(id, pe));
    senderEndpoints.insert(std::make_pair(id, pe));
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
