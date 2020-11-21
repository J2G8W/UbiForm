#include <unistd.h>

#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"

#include "Component.h"



void Component::createNewPairEndpoint(std::string typeOfEndpoint, std::string id){
    EndpointSchema *recvSchema = componentManifest->getReceiverSchema(typeOfEndpoint);
    EndpointSchema *sendSchema = componentManifest->getSenderSchema(typeOfEndpoint);

    PairEndpoint * pe = new PairEndpoint(recvSchema, sendSchema);
    receiverEndpoints.insert(std::make_pair(id, pe));
    senderEndpoints.insert(std::make_pair(id, pe));
}