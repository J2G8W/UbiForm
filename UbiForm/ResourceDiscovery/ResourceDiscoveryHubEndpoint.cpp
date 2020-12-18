
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
        throw NNG_error(rv, "Opening socket for RDH");
    }

    if ((rv = nng_listen(rdSocket, urlInit.c_str(), nullptr, 0)) != 0) {
        throw NNG_error(rv, "Listening on " + urlInit + " for RDH");
    }
    this->rdThread = std::thread(rdBackground, this);
}

void ResourceDiscoveryHubEndpoint::rdBackground(ResourceDiscoveryHubEndpoint * rdhe) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if (nng_recv(rdhe->rdSocket, &buf, &sz, NNG_FLAG_ALLOC) != 0) {
            std::cerr << "NNG error RDH receiving message - " <<  nng_strerror(rv) << std::endl << "CARRY ONE" << std::endl;
            nng_free(buf,sz);
            continue;
        }

        try {
            auto * requestMsg = new SocketMessage(buf);
            SocketMessage * returnMsg = ResourceDiscoveryStore::generateRDResponse(requestMsg, rdhe->rdStore);
            std::string msgText = returnMsg->stringify();
            if ((rv = nng_send(rdhe->rdSocket, (void *) msgText.c_str(), msgText.size() + 1, 0)) != 0) {
                throw NNG_error(rv, "RDHub sending reply");
            }

            delete requestMsg;
            delete returnMsg;
            nng_free(buf,sz);
        }catch (ParsingError &e){
            std::cerr << "Parsing error of request - " << e.what() <<std::endl;
            nng_free(buf,sz);
            continue;
        }catch (ValidationError &e){
            std::cerr << "Validation error of request - " << e.what() <<std::endl;
            nng_free(buf,sz);
            continue;
        }catch (NNG_error &e){
            std::cerr << "NNG error of request - " << e.what() <<std::endl;
            nng_free(buf,sz);
            continue;
        }
    }
}
