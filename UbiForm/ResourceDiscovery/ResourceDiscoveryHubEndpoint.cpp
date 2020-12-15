
#include "ResourceDiscoveryHubEndpoint.h"
#include "../general_functions.h"
#include "../SocketMessage.h"
#include "../ComponentManifest.h"
#include "ComponentRepresentation.h"
#include <nng/protocol/reqrep0/rep.h>
#include <random>

ResourceDiscoveryHubEndpoint::ResourceDiscoveryHubEndpoint(std::string urlInit) :rdSocket() {
    int rv;
    if ((rv = nng_rep0_open(&rdSocket)) != 0) {
        fatal("Failure opening background socket", rv);
    }

    if ((rv = nng_listen(rdSocket, urlInit.c_str(), nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }
    this->rdThread = std::thread(rdBackground, this);
}

void ResourceDiscoveryHubEndpoint::rdBackground(ResourceDiscoveryHubEndpoint * rdhe) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(rdhe->rdSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            fatal("nng_recv", rv);
        }

        try {
            auto * requestMsg = new SocketMessage(buf);

            SocketMessage * returnMsg = generateRDResponse(requestMsg, rdhe);
            std::string msgText = returnMsg->stringify();
            if ((rv = nng_send(rdhe->rdSocket, (void *) msgText.c_str(), msgText.size() + 1, 0)) != 0) {
                throw NNG_error(rv, "Regrep reply");
            }

            delete requestMsg;
            delete returnMsg;
        }catch (std::logic_error &e){
            std::cerr << e.what();
            nng_free(buf,sz);
            continue;
        }
    }
}

SocketMessage * ResourceDiscoveryHubEndpoint::generateRDResponse(SocketMessage * sm, ResourceDiscoveryHubEndpoint *rdhe) {
    std::string request = sm->getString("request");
    auto * returnMsg = new SocketMessage;
    if (request == ADDITION){
        SocketMessage *manifest = sm->getObject("manifest");
        auto *newCR = new ComponentRepresentation(manifest);
        std::minstd_rand0 generator (0);
        std::string id = std::to_string(generator());
        auto p1 = std::make_pair(id, newCR);
        rdhe->componentById.insert(p1);
        returnMsg->addMember("id",id);
    }else if (request == REQUEST_BY_ID){
        std::string id = sm->getString("id");
        if (rdhe->componentById.count(id) > 0){
            std::string component =  rdhe->componentById.at(id)->stringify();
            returnMsg->addMember("component",component);
        }else{
            returnMsg->setNull("component");
        }
    }else if (request == REQUEST_BY_SCHEMA){

    }else if (request == REQUEST_COMPONENTS){
        std::vector<std::string> componentIds;
        for (auto & it : rdhe->componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
    }
    return returnMsg;
}
