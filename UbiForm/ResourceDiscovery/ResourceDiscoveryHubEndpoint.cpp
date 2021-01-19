
#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryHubEndpoint.h"
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include <nng/protocol/reqrep0/rep.h>

void ResourceDiscoveryHubEndpoint::startResourceDiscover(const std::string &baseAddress, int port) {
    replyEndpoint.listenForConnection(baseAddress.c_str(), port);
    backgroundPort = port;
    this->rdThread = std::thread(rdBackground, this);
}

void ResourceDiscoveryHubEndpoint::rdBackground(ResourceDiscoveryHubEndpoint *rdhe) {
    while (true) {
        std::unique_ptr<SocketMessage> request;
        std::unique_ptr<SocketMessage> returnMsg;
        bool receiveError = false;
        try {
            request = rdhe->replyEndpoint.receiveMessage();
        } catch (NngError &e) {
            if (e.errorCode == NNG_ECLOSED) {
                break;
            } else {
                std::cerr << "Resource Discovery Hub - " << e.what() << std::endl;
                break;
            }
        } catch (SocketOpenError &e) {
            std::cerr << e.what() << std::endl;
            break;
        } catch (std::logic_error &e) {
            returnMsg->addMember("error", true);
            returnMsg->addMember("errorMsg", "Receive error " + std::string(e.what()));
            receiveError = true;
        }
        if (!receiveError) {
            try {
                returnMsg = rdhe->rdStore.generateRDResponse(request.get());
                returnMsg->addMember("error", false);
            } catch (ParsingError &e) {
                std::cerr << "Parsing error of request - " << e.what() << std::endl;
                returnMsg = std::unique_ptr<SocketMessage>();
                returnMsg->addMember("error", true);
                returnMsg->addMember("errrorMsg", "Parsing error " + std::string(e.what()));
            } catch (ValidationError &e) {
                std::cerr << "Validation error of request - " << e.what() << "\n\t" << request->stringify()
                          << std::endl;
                returnMsg = std::unique_ptr<SocketMessage>();
                returnMsg->addMember("error", true);
                returnMsg->addMember("errrorMsg", "Validation error " + std::string(e.what()));
            } catch (AccessError &e) {
                std::cerr << "Access error of request - " << e.what() << "\n\t" << request->stringify() << std::endl;
                returnMsg = std::unique_ptr<SocketMessage>();
                returnMsg->addMember("error", true);
                returnMsg->addMember("errrorMsg", "Access error " + std::string(e.what()));
            }
        }

        try {
            rdhe->replyEndpoint.sendMessage(*returnMsg);
        } catch (NngError &e) {
            if (e.errorCode == NNG_ECLOSED) {
                break;
            } else {
                std::cerr << "Resource Discovery Hub - " << e.what() << std::endl;
                break;
            }
        } catch (SocketOpenError &e) {
            break;
        }
    }
}

ResourceDiscoveryHubEndpoint::~ResourceDiscoveryHubEndpoint() {
    replyEndpoint.closeEndpoint();
    nng_msleep(300);

    // We detach our background thread so termination of the thread happens safely
    if (rdThread.joinable()) {
        rdThread.join();
    }
}
