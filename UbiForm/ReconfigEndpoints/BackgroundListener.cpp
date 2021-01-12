#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "BackgroundListener.h"
#include "../Component.h"
#include "../ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

void BackgroundListener::startBackgroundListen(const std::string &baseAddress, int port) {
    replyEndpoint.listenForConnection(baseAddress.c_str(), port);
    std::cout << "Started background listener on " << baseAddress << ":" << port << std::endl;
    backgroundPort = port;
    this->backgroundThread = std::thread(backgroundListen,this);
}

void BackgroundListener::backgroundListen(BackgroundListener * backgroundListener) {
    while (true){
        std::unique_ptr<SocketMessage> request;
        try {
            request = backgroundListener->replyEndpoint.receiveMessage();
        }catch(NngError &e){
            if (e.errorCode == NNG_ECLOSED){
                break;
            }else{
                std::cerr << "Background Listener - " <<  e.what() << std::endl;
                break;
            }
        }catch(SocketOpenError &e){
            break;
        }


        std::unique_ptr<SocketMessage> reply;
        try{
            request->getString("requestType");
            if (request->getString("requestType") == REQ_CONN) {
                backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(*request);
                reply = backgroundListener->handleConnectionRequest((*request));
                backgroundListener->systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(*reply);
            }else if(request->getString("requestType") == ADD_RDH){
                //TODO - validate
                reply = backgroundListener->handleAddRDH(*request);
            }else if(request->getString("requestType") == TELL_REQ_CONN){
                // TODO - validate
                reply = backgroundListener->handleTellCreateConnectionRequest(*request);
            }else if(request->getString("requestType") == CHANGE_ENDPOINT_SCHEMA){
                reply = backgroundListener->handleChangeEndpointRequest(*request);
            }else if(request->getString("requestType") == CREATE_RDH){
                reply = backgroundListener->handleCreateRDHRequest(*request);
            }else if(request->getString("requestType") == CHANGE_MANIFEST){
                reply = backgroundListener->handleChangeManifestRequest(*request);
            }else if(request->getString("requestType") == LOCATIONS_OF_RDH){
                reply = backgroundListener->handleRDHLocationsRequest(*request);
            }else if(request->getString("requestType") == CLOSE_SOCKETS){
                reply = backgroundListener->handleCloseSocketsRequest(*request);
            }else{
                throw ValidationError("requestType had value: " + request->getString("requestType"));
            }
        }catch(ValidationError &e){
            reply = std::make_unique<SocketMessage>();
            reply->addMember("error",true);
            reply->addMember("errorMsg","Validation error - " + std::string(e.what()));
        }catch(std::logic_error &e){
            reply = std::make_unique<SocketMessage>();
            reply->addMember("error",true);
            reply->addMember("errorMsg",e.what());
        }

        try {
            backgroundListener->replyEndpoint.sendMessage(*reply);
        }catch(NngError &e){
            if (e.errorCode == NNG_ECLOSED){
                break;
            }else{
                std::cerr << "Background Listener - " <<  e.what() << std::endl;
                break;
            }
        }catch(SocketOpenError &e){
            break;
        }
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleConnectionRequest(SocketMessage &request){
    try {
        systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(request);
        int port;
        if (request.getString("socketType") == PAIR) {
            port = component->createEndpointAndListen(SocketType::Pair, request.getString("endpointType"));
        } else if (request.getString("socketType") == PUBLISHER) {
            auto existingPublishers = component->getSenderEndpointsByType(request.getString("endpointType"));
            if (existingPublishers->empty()) {
                port = component->createEndpointAndListen(SocketType::Publisher,
                                                                             request.getString("endpointType"));
            }else{
                port = existingPublishers->at(0)->getListenPort();
            }
        }else{
            throw std::logic_error("Not a valid socketType request");
        }
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("port",port);
        reply->addMember("error",false);
        return reply;
    }catch (std::out_of_range &e) {
        throw std::logic_error("No schema of type " + request.getString("endpointType") + " found in this component.");
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleAddRDH(SocketMessage &request){
    auto reply = std::make_unique<SocketMessage>();
    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(request.getString("url"));
    reply->addMember("error",false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleTellCreateConnectionRequest(SocketMessage &request){
    component->getBackgroundRequester().requestAndCreateConnection(
            request.getString("remoteAddress"), request.getInteger("port"), request.getString("reqEndpointType"),
            request.getString("remoteEndpointType"));
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error",false);
    return reply;

}

std::unique_ptr<SocketMessage> BackgroundListener::handleChangeEndpointRequest(SocketMessage &request){
    std::unique_ptr<SocketMessage> sendSchema;
    std::unique_ptr<SocketMessage> receiveSchema;

    std::shared_ptr<EndpointSchema> esSendSchema;
    std::shared_ptr<EndpointSchema> esReceiveSchema;


    if (request.isNull("sendSchema")){
        esSendSchema = nullptr;
    }else{
        sendSchema = request.getMoveObject("sendSchema");
        esSendSchema = std::make_shared<EndpointSchema>(*sendSchema);
    }

    if (request.isNull("receiveSchema")){
        esReceiveSchema = nullptr;
    }else{
        receiveSchema = request.getMoveObject("receiveSchema");
        esReceiveSchema = std::make_shared<EndpointSchema>(*receiveSchema);
    }

    component->getComponentManifest().addEndpoint(static_cast<SocketType>(request.getInteger("socketType")),
                                                   request.getString("endpointType"), esReceiveSchema, esSendSchema);

    std::cout << "Endpoint of type: " << request.getString("endpointType") << " changed" << std::endl;
    component->getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();

    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error",false);
    reply->addMember("errorMsg", "All good");

    return reply;

}

std::unique_ptr<SocketMessage> BackgroundListener::handleCreateRDHRequest(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();

    int port = component->startResourceDiscoveryHub();
    reply->addMember("port",port);
    reply->addMember("error",false);

    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleChangeManifestRequest(SocketMessage &request){
    auto reply = std::make_unique<SocketMessage>();
    auto manifestObject = request.getMoveObject("newManifest");
    component->specifyManifest(manifestObject.get());
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleRDHLocationsRequest(SocketMessage &request){
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("locations",component->getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs());
    reply->addMember("error",false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCloseSocketsRequest(SocketMessage &request){
    auto reply = std::make_unique<SocketMessage>();
    component->closeSocketsOfType(request.getString("endpointType"));
    reply->addMember("error",false);
    return reply;
}

BackgroundListener::~BackgroundListener() {
    replyEndpoint.closeSocket();
    nng_msleep(300);

    // We detach our background thread so termination of the thread happens safely
    if(backgroundThread.joinable()) {
        backgroundThread.join();
    }
}