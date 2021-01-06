#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "BackgroundListener.h"
#include "../Component.h"
#include "../ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

void BackgroundListener::startBackgroundListen(const std::string& listenAddress) {
    replyEndpoint.listenForConnection(listenAddress.c_str());
    backgroundListenAddress = listenAddress;
    this->backgroundThread = std::thread(backgroundListen,this);
}

void BackgroundListener::backgroundListen(BackgroundListener * backgroundListener) {
    while (true){
        auto request = backgroundListener->replyEndpoint.receiveMessage();
        try{
            if (request->getString("requestType") == REQ_CONN) {
                backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(*request);
                auto reply = backgroundListener->handleConnectionRequest((*request));
                backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(*reply);
                backgroundListener->replyEndpoint.sendMessage(*reply);
            }else if(request->getString("requestType") == ADD_RDH){
                //TODO - validate
                auto reply = backgroundListener->handleAddRDH(*request);
                backgroundListener->replyEndpoint.sendMessage(*reply);
            }else if(request->getString("requestType") == TELL_REQ_CONN){
                // TODO - validate
                auto reply = backgroundListener->handleTellCreateConnectionRequest(*request);
                backgroundListener->replyEndpoint.sendMessage(*reply);
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

std::unique_ptr<SocketMessage> BackgroundListener::handleConnectionRequest(SocketMessage &request){
    try {
        systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(request);
        std::string url;
        if (request.getString("socketType") == PAIR) {
            url = component->createEndpointAndListen(SocketType::Pair, request.getString("endpointType"));
        } else if (request.getString("socketType") == PUBLISHER) {
            auto existingPublishers = component->getSenderEndpointsByType(request.getString("endpointType"));
            if (existingPublishers->empty()) {
                url = component->createEndpointAndListen(SocketType::Publisher,
                                                                             request.getString("endpointType"));
            }else{
                url = existingPublishers->at(0)->getListenUrl();
            }
        }else{
            throw std::logic_error("Not a valid socketType request");
        }
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("url",url);
        return reply;
    }catch (std::out_of_range &e) {
        std::cerr << "No schema of type " << request.getString("endpointType") << " found in this component."
                  << std::endl;
        // Return a simple reply, any errors on send are IGNORED
        auto reply = std::make_unique<SocketMessage>();
        reply->setNull("url");
        return reply;
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleAddRDH(SocketMessage &request){
    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(request.getString("url"));
    return std::make_unique<SocketMessage>();
}

std::unique_ptr<SocketMessage> BackgroundListener::handleTellCreateConnectionRequest(SocketMessage &request){
    try {
        component->requestAndCreateConnection(
                request.getString("remoteAddress"), request.getString("reqEndpointType"),
                request.getString("remoteEndpointType"));
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("error",false);
        reply->addMember("errorMsg", "All good");
        return reply;
    }catch(std::logic_error &e){
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("error",true);
        reply->addMember("errorMsg", e.what());
        return reply;
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleAddEndpointRequest(SocketMessage &request){
    SocketMessage* sendSchema;
    SocketMessage* receiveSchema;
    try{
        std::shared_ptr<EndpointSchema> esSendSchema;
        std::shared_ptr<EndpointSchema> esReceiveSchema;


        if (request.isNull("sendSchema")){
            sendSchema = nullptr;
            esSendSchema = nullptr;
        }
        else{
            sendSchema = request.getObject("sendSchema");
            esSendSchema = std::make_shared<EndpointSchema>(*sendSchema);
        }

        if (request.isNull("receiveSchema")){
            receiveSchema = nullptr;
            esSendSchema = nullptr;
        }
        else{
            receiveSchema = request.getObject("receiveSchema");
            esReceiveSchema = std::make_shared<EndpointSchema>(*receiveSchema);
        }

        component->getComponentManifest()->addEndpoint(static_cast<SocketType>(request.getInteger("socketType")),
                                                       request.getString("endpointType"), esReceiveSchema, esSendSchema);

        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("error",false);
        reply->addMember("errorMsg", "All good");

        delete sendSchema;
        delete receiveSchema;

        return reply;
    }catch(std::logic_error &e){
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("error",true);
        reply->addMember("errorMsg", e.what());

        delete sendSchema;
        delete receiveSchema;
        return reply;
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

