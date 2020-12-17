
#include "ResourceDiscoveryHubEndpoint.h"
#include "../general_functions.h"
#include "../SocketMessage.h"
#include "../ComponentManifest.h"
#include "ComponentRepresentation.h"
#include <nng/protocol/reqrep0/rep.h>
#include <random>

void ResourceDiscoveryHubEndpoint::startResourceDiscover(std::string urlInit){
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
            SocketMessage * returnMsg = ResourceDiscoveryStore::generateRDResponse(requestMsg, rdhe->rdStore);
            std::string msgText = returnMsg->stringify();
            if ((rv = nng_send(rdhe->rdSocket, (void *) msgText.c_str(), msgText.size() + 1, 0)) != 0) {
                throw NNG_error(rv, "Regrep reply");
            }

            delete requestMsg;
            delete returnMsg;
            nng_free(buf,sz);
        }catch (std::logic_error &e){
            std::cerr << e.what() << std::endl;
            nng_free(buf,sz);
            continue;
        }
    }
}
