#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/ReconfigurationEndpoints/BackgroundListener.h"
#include "../../include/UbiForm/Component.h"
#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

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
                std::string requestType = request->getString("requestType");
                if (requestType == BACKGROUND_CREATE_AND_LISTEN) {
                    reply = backgroundListener->handleCreateAndListen((*request));
                } else if (requestType == BACKGROUND_CREATE_AND_DIAL) {
                    reply = backgroundListener->handleCreateAndDial(*request);
                } else if (requestType == BACKGROUND_ADD_RDH) {
                    reply = backgroundListener->handleAddRDH(*request);
                } else if (requestType == BACKGROUND_REMOVE_RDH) {
                    reply = backgroundListener->handleRemoveRDH(*request);
                } else if (requestType == BACKGROUND_3RD_PARTY_REMOTE_LISTEN_THEN_DIAL) {
                    reply = backgroundListener->handle3rdPartyRemoteListenThenDial(*request);
                } else if (requestType == BACKGROUND_3RD_PARTY_LOCAL_LISTEN_THEN_REMOTE_DIAL) {
                    reply = backgroundListener->handle3rdPartyLocalListenThenRemoteDial(*request);
                } else if (requestType == BACKGROUND_CHANGE_ENDPOINT_SCHEMA) {
                    reply = backgroundListener->handleChangeEndpointRequest(*request);
                } else if (requestType == BACKGROUND_CREATE_RDH) {
                    reply = backgroundListener->handleCreateRDHRequest(*request);
                } else if (requestType == BACKGROUND_CHANGE_MANIFEST) {
                    reply = backgroundListener->handleChangeManifestRequest(*request);
                } else if (requestType == BACKGROUND_GET_LOCATIONS_OF_RDH) {
                    reply = backgroundListener->handleRDHLocationsRequest(*request);
                } else if (requestType == BACKGROUND_CLOSE_SOCKETS) {
                    reply = backgroundListener->handleCloseSocketsRequest(*request);
                } else if (requestType == BACKGROUND_CLOSE_ENDPOINT_BY_ID) {
                    reply = backgroundListener->handleCloseEndpointByIdRequest(*request);
                } else if (requestType == BACKGROUND_CLOSE_RDH) {
                    reply = backgroundListener->handleCloseRDH(*request);
                } else if (requestType == BACKGROUND_REQUEST_ENDPOINT_INFO) {
                    reply = backgroundListener->handleEndpointInfoRequest(*request);
                } else if (requestType == BACKGROUND_GET_COMPONENT_MANIFEST) {
                    reply = backgroundListener->handleManifestRequest(*request);
                } else {
                    throw ValidationError("requestType had value: " + requestType);
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

std::unique_ptr<SocketMessage> BackgroundListener::handleCreateAndListen(SocketMessage &request) {
    try {
        systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(request);

        auto socketType = component->getComponentManifest().getSocketType(request.getString("endpointType"));
        int port = component->createEndpointAndListen(request.getString("endpointType"));

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

std::unique_ptr<SocketMessage> BackgroundListener::handleRemoveRDH(SocketMessage &request){
    auto reply = std::make_unique<SocketMessage>();
    component->getResourceDiscoveryConnectionEndpoint().deRegisterFromHub(request.getString("url"));
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handle3rdPartyRemoteListenThenDial(SocketMessage &request) {
    component->getBackgroundRequester().requestRemoteListenThenDial(
            request.getString("remoteAddress"),
            request.getInteger("port"),
            request.getString("reqEndpointType"),
            request.getString("remoteEndpointType"));
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;

}

std::unique_ptr<SocketMessage> BackgroundListener::handle3rdPartyLocalListenThenRemoteDial(SocketMessage &request){
    component->getBackgroundRequester().localListenThenRequestRemoteDial(request.getString("dialerAddress"),
                                                                         request.getString("listenEndpointType"),
                                                                         request.getString("dialEndpointType"));
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
    component->closeAndInvalidateSocketsOfType(request.getString("endpointType"));
    reply->addMember("error", false);
    return reply;
}


std::unique_ptr<SocketMessage> BackgroundListener::handleCloseRDH(SocketMessage &sm) {
    component->closeResourceDiscoveryHub();
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCreateAndDial(SocketMessage &sm) {
    std::string socketType = component->getComponentManifest().getSocketType(sm.getString("endpointType"));
    bool success = false;
    for (const auto &url: sm.getArray<std::string>("dialUrls")) {
        try {
            component->createEndpointAndDial(sm.getString("endpointType"), url);
            success = true;
            break;
        } catch (NngError &e) {
            continue;
        }
    }
    if (!success) {
        throw std::logic_error("Could not dial any of the given urls");
    }

    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleEndpointInfoRequest(SocketMessage &request) {
    std::unique_ptr<SocketMessage> reply = std::make_unique<SocketMessage>();
    std::vector<std::string> endpointTypes = component->getComponentManifest().getAllEndpointTypes();
    std::vector<std::unique_ptr<SocketMessage>> endpoints;
    for (const auto &type:endpointTypes) {
        auto endpointObjects = component->getEndpointsByType(type);
        for (const auto &endpoint: *endpointObjects) {
            std::unique_ptr<SocketMessage> mini = std::make_unique<SocketMessage>();
            mini->addMember("id", endpoint->getEndpointId());
            mini->addMember("endpointType", type);
            mini->addMember("socketType", component->getComponentManifest().getSocketType(type));
            try {
                if (endpoint->getEndpointState() == EndpointState::Listening) {
                    mini->addMember("listenPort", component->castToDataSenderEndpoint(endpoint)->getListenPort());
                } else if (endpoint->getEndpointState() == EndpointState::Dialed) {
                    mini->addMember("dialUrl", component->castToDataReceiverEndpoint(endpoint)->getDialUrl());
                }
            }catch(AccessError &e){
                // EMPTY - error made with casting
            }

            endpoints.push_back(std::move(mini));

        }
    }
    reply->addMoveArrayOfObjects("endpoints", endpoints);
    reply->addMember("error", false);

    return reply;
}

std::unique_ptr<SocketMessage> BackgroundListener::handleCloseEndpointByIdRequest(SocketMessage &re) {
    component->closeAndInvalidateSocketById(re.getString("endpointId"));
    auto reply = std::make_unique<SocketMessage>();
    reply->addMember("error", false);
    return reply;
}


BackgroundListener::~BackgroundListener() {
    replyEndpoint.closeEndpoint();
    nng_msleep(300);

    // We detach our background thread so termination of the thread happens safely
    if (backgroundThread.joinable()) {
        backgroundThread.join();
    }
}

std::unique_ptr<SocketMessage> BackgroundListener::handleManifestRequest(SocketMessage &request) {
    std::unique_ptr<SocketMessage> reply = std::make_unique<SocketMessage>();
    reply->addMoveObject("manifest", component->getComponentManifest().getSocketMessageCopy());
    reply->addMember("error",false);
    return reply;
}
