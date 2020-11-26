#include <unistd.h>

#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"

#include "Component.h"

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
