#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include "../../include/UbiForm/ReconfigurationEndpoints/BackgroundListener.h"
#include "../../include/UbiForm/Component.h"
#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

void BackgroundListener::startBackgroundListen(const std::string &baseAddress, int port) {
    replyEndpoint.listenForConnection(baseAddress, port);
    if(VIEW_STD_OUTPUT) std::cout << "Started background listener on " << baseAddress << ":" << port << std::endl;
    backgroundPort = port;
    this->backgroundThread = std::thread(backgroundListen, this);
}

void BackgroundListener::backgroundListen(BackgroundListener *backgroundListener) {
    while (true) {
        std::unique_ptr<EndpointMessage> request;
        std::unique_ptr<EndpointMessage> reply;
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
        } catch (EndpointOpenError &e) {
            // If the endpoint is no longer open then we just end our background process
            break;
        } catch (std::logic_error &e) {
            // For validation errors or something similar, we return the error messahe
            validRequest = false;
            reply = std::make_unique<EndpointMessage>();
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
                } else if (requestType == BACKGROUND_CLOSE_ENDPOINTS) {
                    reply = backgroundListener->handleCloseEndpointsRequest(*request);
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
                reply = std::make_unique<EndpointMessage>();
                reply->addMember("error", true);
                reply->addMember("errorMsg", "Validation error - " + std::string(e.what()));
            } catch (std::logic_error &e) {
                reply = std::make_unique<EndpointMessage>();
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
        } catch (EndpointOpenError &e) {
            break;
        }
    }
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleCreateAndListen(EndpointMessage &request) {
    try {
        systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(request);

        auto connectionParadigm = component->getComponentManifest().getConnectionParadigm(
                request.getString("endpointType"));
        int port = component->createEndpointAndListen(request.getString("endpointType"));

        auto reply = std::make_unique<EndpointMessage>();
        reply->addMember("port", port);
        reply->addMember("error", false);
        return reply;
    } catch (std::out_of_range &e) {
        throw std::logic_error("No schema of type " + request.getString("endpointType") + " found in this component.");
    }
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleAddRDH(EndpointMessage &request) {
    auto reply = std::make_unique<EndpointMessage>();
    component->getResourceDiscoveryConnectionEndpoint().registerWithHub(request.getString("url"));
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleRemoveRDH(EndpointMessage &request){
    auto reply = std::make_unique<EndpointMessage>();
    component->getResourceDiscoveryConnectionEndpoint().deRegisterFromHub(request.getString("url"));
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handle3rdPartyRemoteListenThenDial(EndpointMessage &request) {
    component->getBackgroundRequester().requestRemoteListenThenDial(
            request.getString("remoteAddress"),
            request.getInteger("port"),
            request.getString("reqEndpointType"),
            request.getString("remoteEndpointType"));
    auto reply = std::make_unique<EndpointMessage>();
    reply->addMember("error", false);
    return reply;

}

std::unique_ptr<EndpointMessage> BackgroundListener::handle3rdPartyLocalListenThenRemoteDial(EndpointMessage &request){
    component->getBackgroundRequester().localListenThenRequestRemoteDial(request.getString("dialerAddress"),
                                                                         request.getString("listenEndpointType"),
                                                                         request.getString("dialEndpointType"));
    auto reply = std::make_unique<EndpointMessage>();
    reply->addMember("error", false);
    return reply;

}

std::unique_ptr<EndpointMessage> BackgroundListener::handleChangeEndpointRequest(EndpointMessage &request) {
    std::unique_ptr<EndpointMessage> sendSchema;
    std::unique_ptr<EndpointMessage> receiveSchema;

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

    component->getComponentManifest().addEndpoint(static_cast<ConnectionParadigm>(request.getInteger("connectionParadigm")),
                                                  request.getString("endpointType"), esReceiveSchema, esSendSchema);

    if(VIEW_STD_OUTPUT) std::cout << "Endpoint of type: " << request.getString("endpointType") << " changed" << std::endl;
    component->getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();

    auto reply = std::make_unique<EndpointMessage>();
    reply->addMember("error", false);
    reply->addMember("errorMsg", "All good");

    return reply;

}

std::unique_ptr<EndpointMessage> BackgroundListener::handleCreateRDHRequest(EndpointMessage &request) {
    auto reply = std::make_unique<EndpointMessage>();

    int port = component->startResourceDiscoveryHub();
    reply->addMember("port", port);
    reply->addMember("error", false);

    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleChangeManifestRequest(EndpointMessage &request) {
    auto reply = std::make_unique<EndpointMessage>();
    auto manifestObject = request.getMoveObject("newManifest");
    component->specifyManifest(manifestObject.get());
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleRDHLocationsRequest(EndpointMessage &request) {
    auto reply = std::make_unique<EndpointMessage>();
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

std::unique_ptr<EndpointMessage> BackgroundListener::handleCloseEndpointsRequest(EndpointMessage &request) {
    auto reply = std::make_unique<EndpointMessage>();
    component->closeAndInvalidateEndpointsOfType(request.getString("endpointType"));
    reply->addMember("error", false);
    return reply;
}


std::unique_ptr<EndpointMessage> BackgroundListener::handleCloseRDH(EndpointMessage &sm) {
    component->closeResourceDiscoveryHub();
    auto reply = std::make_unique<EndpointMessage>();
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleCreateAndDial(EndpointMessage &sm) {
    std::string connectionParadigm = component->getComponentManifest().getConnectionParadigm(
            sm.getString("endpointType"));
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

    auto reply = std::make_unique<EndpointMessage>();
    reply->addMember("error", false);
    return reply;
}

std::unique_ptr<EndpointMessage> BackgroundListener::handleEndpointInfoRequest(EndpointMessage &request) {
    std::unique_ptr<EndpointMessage> reply = std::make_unique<EndpointMessage>();
    std::vector<std::string> endpointTypes = component->getComponentManifest().getAllEndpointTypes();
    std::vector<std::unique_ptr<EndpointMessage>> endpoints;
    for (const auto &type:endpointTypes) {
        auto endpointObjects = component->getEndpointsByType(type);
        for (const auto &endpoint: *endpointObjects) {
            std::unique_ptr<EndpointMessage> mini = std::make_unique<EndpointMessage>();
            mini->addMember("id", endpoint->getEndpointId());
            mini->addMember("endpointType", type);
            mini->addMember("connectionParadigm", component->getComponentManifest().getConnectionParadigm(type));
            mini->addMember("endpointState",convertEndpointState(endpoint->getEndpointState()));
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

std::unique_ptr<EndpointMessage> BackgroundListener::handleCloseEndpointByIdRequest(EndpointMessage &re) {
    component->closeAndInvalidateEndpointsById(re.getString("endpointId"));
    auto reply = std::make_unique<EndpointMessage>();
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

std::unique_ptr<EndpointMessage> BackgroundListener::handleManifestRequest(EndpointMessage &request) {
    std::unique_ptr<EndpointMessage> reply = std::make_unique<EndpointMessage>();
    reply->addMoveObject("manifest", component->getComponentManifest().getEndpointMessageCopy());
    reply->addMember("error",false);
    return reply;
}
