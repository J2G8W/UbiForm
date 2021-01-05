#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "BackgroundListener.h"
#include "../Component.h"

void BackgroundListener::startBackgroundListen(const std::string& listenAddress) {
    int rv;

    if ((rv = nng_listen(backgroundSocket, listenAddress.c_str(), nullptr, 0)) != 0) {
        throw NngError(rv, "Listening on " + listenAddress);
    }
    backgroundListenAddress = listenAddress;
    this->backgroundThread = std::thread(backgroundListen,this);

}

void BackgroundListener::backgroundListen(BackgroundListener * backgroundListener) {
    int rv;
    while (true){
        char *buf = nullptr;
        size_t sz;
        if ((rv = nng_recv(backgroundListener->backgroundSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
            std::cerr << "NNG error receiving component request - " << nng_strerror(rv) << std::endl;
            continue;
        }
        SocketMessage sm(buf);
        nng_free(buf, sz);
        try {
            backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);
            std::string url;
            if (sm.getString("socketType") == PAIR) {
                url = backgroundListener->component->createEndpointAndListen(SocketType::Pair,
                                                                             sm.getString("endpointType"));
            } else if (sm.getString("socketType") == PUBLISHER) {
                auto existingPublishers = backgroundListener->component->getSenderEndpointsByType(sm.getString("endpointType"));
                if (existingPublishers->empty()) {
                    url = backgroundListener->component->createEndpointAndListen(SocketType::Publisher,
                                                                                 sm.getString("endpointType"));
                }else{
                    url = existingPublishers->at(0)->getListenUrl();
                }
            }else{
                throw std::logic_error("Not a valid socketType request");
            }
            SocketMessage reply;
            reply.addMember("url",url);
            backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(reply);
            std::string replyText = reply.stringify();

            // Send reply on regrep with url for the component to dial
            if ((rv = nng_send(backgroundListener->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0)) != 0) {
                throw NngError(rv, "Replying to Pair creation request");
            }
        }catch (std::out_of_range &e){
            std::cerr << "No schema of type " << sm.getString("endpointType") << " found in this component." <<  std::endl;
            // Return a simple reply, any errors on send are IGNORED
            SocketMessage reply;
            reply.setNull("url");
            backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(reply);
            std::string replyText = reply.stringify();
            if (nng_send(backgroundListener->backgroundSocket, (void *) replyText.c_str(), replyText.size() + 1, 0) != 0) {
                // IGNORE
            }
        }catch(ValidationError &e){
            std::cerr << "Invalid creation request - " << e.what() <<std::endl;
        }catch(NngError &e){
            std::cerr << "NNG error in handling creation request - " << e.what() <<std::endl;
        }catch(std::logic_error &e){
            std::cerr << e.what() << std::endl;
        }
    }
}

BackgroundListener::~BackgroundListener() {
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

