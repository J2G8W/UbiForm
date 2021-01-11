
#include "ResourceDiscoveryHubEndpoint.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include <nng/protocol/reqrep0/rep.h>

void ResourceDiscoveryHubEndpoint::startResourceDiscover(const std::string &baseAddress, int port) {
    replyEndpoint.listenForConnection(baseAddress.c_str(), port);
    backgroundPort = port;
    this->rdThread = std::thread(rdBackground, this);
}

void ResourceDiscoveryHubEndpoint::rdBackground(ResourceDiscoveryHubEndpoint * rdhe) {
    while (true){
        std::unique_ptr<SocketMessage> request;
        try {
            request = rdhe->replyEndpoint.receiveMessage();
        }catch(NngError &e){
            if (e.errorCode == NNG_ECLOSED){
                break;
            }else{
                std::cerr << "Resource Discovery Hub - " <<  e.what() << std::endl;
                break;
            }
        }catch(SocketOpenError &e){
            break;
        }
        std::unique_ptr<SocketMessage> returnMsg;
        try {
            returnMsg = std::unique_ptr<SocketMessage>(ResourceDiscoveryStore::generateRDResponse(request.get(), rdhe->rdStore));
        }catch (ParsingError &e){
            std::cerr << "Parsing error of request - " << e.what() <<std::endl;
            continue;
        }catch (ValidationError &e){
            std::cerr << "Validation error of request - " << e.what() << "\n\t" << request->stringify() << std::endl;
            continue;
        }

        try{
            rdhe->replyEndpoint.sendMessage(*returnMsg);
        }catch(NngError &e){
            if (e.errorCode == NNG_ECLOSED){
                break;
            }else{
                std::cerr << "Resource Discovery Hub - " <<  e.what() << std::endl;
                continue;
            }
        }catch(SocketOpenError &e){
            break;
        }
    }
}

ResourceDiscoveryHubEndpoint::~ResourceDiscoveryHubEndpoint() {
    replyEndpoint.closeSocket();
    nng_msleep(300);

    // We detach our background thread so termination of the thread happens safely
    if(rdThread.joinable()) {
        rdThread.join();
    }
}
