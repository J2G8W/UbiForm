#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "BackgroundListener.h"
#include "../Component.h"
#include "../ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

void BackgroundListener::startBackgroundListen(const std::string &baseAddress, int port) {
    replyEndpoint.listenForConnection(baseAddress.c_str(), port);
    std::cout << "Started background listener on " << baseAddress << ":" << port << std::endl;
    backgroundPort = port;
    this->backgroundThread = std::thread(backgroundListen, this);
}

void BackgroundListener::backgroundListen(BackgroundListener *backgroundListener) {
    while (true) {
        std::unique_ptr<SocketMessage> request;
        std::unique_ptr<SocketMessage> reply;
        bool validRequest = true;
        try {
            request = backgroundListener->replyEndpoint.receiveMessage();
        } catch (NngError &e) {
            // For NNG errors we just gracefully end our background process
            if (e.errorCode == NNG_ECLOSED) {
                break;
            } else {
                std::cerr << "Background Listener - " << e.what() << std::endl;
                break;
            }
        } catch (SocketOpenError &e) {
            // If the socket is no longer open then we just end our background process
            break;
        } catch (std::logic_error &e) {
            // For validation errors or something similar, we return the error messahe
            validRequest = false;
            reply = std::make_unique<SocketMessage>();
            reply->addMember("error", true);
            reply->addMember("errorMsg", e.what());
        }


        if (validRequest) {
            try {
                request->getString("requestType");
                if (request->getString("requestType") == BACKGROUND_REQUEST_CONNECTION) {
                    backgroundListener->systemSchemas.getSystemSchema(
                            SystemSchemaName::endpointCreationRequest).validate(*request);
                    reply = backgroundListener->handleConnectionRequest((*request));
                    backgroundListener->systemSchemas.getSystemSchema(
                            SystemSchemaName::endpointCreationResponse).validate(*reply);
                } else if (request->getString("requestType") == BACKGROUND_ADD_RDH) {
                    reply = backgroundListener->handleAddRDH(*request);
                } else if (request->getString("requestType") == BACKGROUND_TELL_TO_REQUEST_CONNECTION) {
                    reply = backgroundListener->handleTellCreateConnectionRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_CHANGE_ENDPOINT_SCHEMA) {
                    reply = backgroundListener->handleChangeEndpointRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_CREATE_RDH) {
                    reply = backgroundListener->handleCreateRDHRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_CHANGE_MANIFEST) {
                    reply = backgroundListener->handleChangeManifestRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_GET_LOCATIONS_OF_RDH) {
                    reply = backgroundListener->handleRDHLocationsRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_CLOSE_SOCKETS) {
                    reply = backgroundListener->handleCloseSocketsRequest(*request);
                } else if (request->getString("requestType") == BACKGROUND_CLOSE_RDH) {
                    reply = backgroundListener->handleCloseRDH(*request);
                } else {
                    throw ValidationError("requestType had value: " + request->getString("requestType"));
                }
            } catch (ValidationError &e) {
                reply = std::make_unique<SocketMessage>();
                reply->addMember("error", true);
                reply->addMember("errorMsg", "Validation error - " + std::string(e.what()));
            } catch (std::logic_error &e) {
                reply = std::make_unique<SocketMessage>();
                reply->addMember("error", true);
                reply->addMember("errorMsg", e.what());
            }
        }

        try {
            backgroundListener->replyEndpoint.sendMessage(*reply);
        } catch (NngError &e) {
            if (e.errorCode == NNG_ECLOSED) {
                break;
            } else {
                std::cerr << "Background Listener - " << e.what() << std::endl;
                break;
            }
        } catch (SocketOpenError &e) {
            break;
        }
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleConnectionRequest(SocketMessage &request) {
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
            } else {
                port = existingPublishers->at(0)->getListenPort();
            }
        } else {
            throw std::logic_error("Not a valid socketType request");
        }
        auto reply = std::make_unique<SocketMessage>();
        reply->addMember("port", port);
        reply->addMember("error", false);
        return reply;
    } catch (std::out_of_range &e) {
        throw std::logic_error("No schema of type " + request.getString("endpointType") + " found in this component.");
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleAddRDH(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();
    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(request.getString("url"));
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleTellCreateConnectionRequest(SocketMessage &request) {
    component->getBackgroundRequester().requestAndCreateConnection(
            request.getString("remoteAddress"), request.getInteger("port"), request.getString("reqEndpointType"),
            request.getString("remoteEndpointType"));
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;

}

std::unique_ptr<SocketMessage> BackgroundListener::handleChangeEndpointRequest(SocketMessage &request) {
    std::unique_ptr<SocketMessage> sendSchema;
    std::unique_ptr<SocketMessage> receiveSchema;

    std::shared_ptr<EndpointSchema> esSendSchema;
    std::shared_ptr<EndpointSchema> esReceiveSchema;


    if (request.isNull("sendSchema")) {
        esSendSchema = nullptr;
    } else {
        sendSchema = request.getMoveObject("sendSchema");
        esSendSchema = std::make_shared<EndpointSchema>(*sendSchema);
    }

    if (request.isNull("receiveSchema")) {
        esReceiveSchema = nullptr;
    } else {
        receiveSchema = request.getMoveObject("receiveSchema");
        esReceiveSchema = std::make_shared<EndpointSchema>(*receiveSchema);
    }

    component->getComponentManifest().addEndpoint(static_cast<SocketType>(request.getInteger("socketType")),
                                                  request.getString("endpointType"), esReceiveSchema, esSendSchema);

    std::cout << "Endpoint of type: " << request.getString("endpointType") << " changed" << std::endl;
    component->getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();

    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    reply->addMember("errorMsg", "All good");

    return reply;

}

std::unique_ptr<SocketMessage> BackgroundListener::handleCreateRDHRequest(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();

    int port = component->startResourceDiscoveryHub();
    reply->addMember("port", port);
    reply->addMember("error", false);

    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleChangeManifestRequest(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();
    auto manifestObject = request.getMoveObject("newManifest");
    component->specifyManifest(manifestObject.get());
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleRDHLocationsRequest(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();
    auto locations = component->getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs();

    bool selfRDH = false;
    auto it = locations.begin();
    while (it != locations.end()) {
        if (it->rfind(component->getSelfAddress(), 0) == 0) {
            selfRDH = true;
            it = locations.erase(it);
        } else {
            it++;
        }

    }
    if (selfRDH) {
        for (const auto &url: component->getAllAddresses()) {
            std::string changedUrl = url + ":" + std::to_string(component->getResourceDiscoveryHubPort());
            locations.push_back(changedUrl);
        }
    }

    reply->addMember("locations", locations);
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCloseSocketsRequest(SocketMessage &request) {
    auto reply = std::make_unique<SocketMessage>();
    component->closeSocketsOfType(request.getString("endpointType"));
    reply->addMember("error", false);
    return reply;
}

BackgroundListener::~BackgroundListener() {
    replyEndpoint.closeSocket();
    nng_msleep(300);

    // We detach our background thread so termination of the thread happens safely
    if (backgroundThread.joinable()) {
        backgroundThread.join();
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCloseRDH(SocketMessage &sm) {
    component->closeResourceDiscoveryHub();
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCreateAndDial(SocketMessage& sm) {
    std::string socketType = component->getComponentManifest().getSocketType(sm.getString("endpointType"));
    component->createEndpointAndDial(socketType,sm.getString("endpointType"), sm.getString("dialUrl"));
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error",false);
    return reply;
}
