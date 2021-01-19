#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
#include "../../include/UbiForm/Component.h"
#include <nng/protocol/reqrep0/req.h>

std::unique_ptr<SocketMessage>
ResourceDiscoveryConnEndpoint::sendRequest(const std::string &url, SocketMessage &request) {

    requestEndpoint.dialConnection(url.c_str());
    requestEndpoint.sendMessage(request);
    auto reply = requestEndpoint.receiveMessage();
    if (reply->getBoolean("error")) {
        if (reply->hasMember("errorMsg")) {
            throw RemoteError("Error with request: " + reply->getString("errorMsg"), url);
        } else {
            throw RemoteError("Error with request, no error message", url);
        }
    }
    return reply;
}

std::unique_ptr<SocketMessage> ResourceDiscoveryConnEndpoint::generateRegisterRequest() {
    std::unique_ptr<SocketMessage> request = std::make_unique<SocketMessage>();
    request->addMember("request", RESOURCE_DISCOVERY_ADD_COMPONENT);
    auto sm = component->getComponentManifest().getSocketMessageCopy();
    sm->addMember("urls", component->getAllAddresses());
    sm->addMember("port", component->getBackgroundPort());

    request->addMoveObject("manifest", std::move(sm));
    return request;
}

void ResourceDiscoveryConnEndpoint::registerWithHub(const std::string &url) {
    std::unique_ptr<SocketMessage> request = generateRegisterRequest();

    systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*request);
    std::unique_ptr<SocketMessage> reply;
    try {
        reply = sendRequest(url, *request);
        systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
        std::cout << "Registered successfully with: " << url << " id is " << reply->getString("newID") << std::endl;
    } catch (std::logic_error &e) {
        throw;
    }

    resourceDiscoveryHubs.insert(std::pair<std::string, std::string>(url, reply->getString("newID")));
}

std::vector<std::string> ResourceDiscoveryConnEndpoint::getComponentIdsFromHub(const std::string &url) {
    SocketMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_COMPONENTS);

    systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(request);
    std::unique_ptr<SocketMessage> reply;

    try {
        reply = sendRequest(url, request);
        systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*reply);
    } catch (std::logic_error &e) {
        std::cerr << "Error getting component ids from " << url << "\n\t" << e.what() << std::endl;
        return std::vector<std::string>();
    }

    // This is a copy constructor
    return reply->getArray<std::string>("components");
}

std::unique_ptr<ComponentRepresentation>
ResourceDiscoveryConnEndpoint::getComponentById(const std::string &url, const std::string &id) {
    SocketMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_ID);
    request.addMember("id", id);

    systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(request);


    std::unique_ptr<SocketMessage> reply;
    try {
        reply = sendRequest(url, request);
        systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*reply);
    } catch (std::logic_error &e) {
        throw;
    }

    // Here we check if the ComponentRepresentation returned is a valid ComponentRepresentation
    try {
        // We copy compRep when making the ComponentRepresentation object anyway
        auto compRep = reply->getMoveObject("component");
        auto componentRepresentation = std::make_unique<ComponentRepresentation>(compRep.get(), systemSchemas);
        return componentRepresentation;
    } catch (std::logic_error &e) {
        std::cerr << "Malformed reply from RDH" << std::endl;
        throw ValidationError(e.what());
    }
}

std::unique_ptr<SocketMessage>
ResourceDiscoveryConnEndpoint::generateFindBySchemaRequest(const std::string &endpointType,
                                                           std::map<std::string, std::string> &otherValues) {
    std::unique_ptr<SocketMessage> request = std::make_unique<SocketMessage>();
    request->addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA);

    // We want the schema we get back to be a data SENDER
    request->addMember("dataReceiverEndpoint", false);

    // We want our schema to be receiving data
    auto schema = component->getComponentManifest().getSchemaObject(endpointType, true);

    request->addMoveObject("schema", std::move(schema));

    auto specialProperties = std::make_unique<SocketMessage>();
    for (auto &keyValuePair : otherValues) {
        specialProperties->addMember(keyValuePair.first, keyValuePair.second);
    }
    request->addMoveObject("specialProperties", std::move(specialProperties));

    return request;
}

std::vector<std::unique_ptr<SocketMessage>>
ResourceDiscoveryConnEndpoint::getComponentsBySchema(const std::string &endpointType,
                                                     std::map<std::string, std::string> &otherValues) {
    std::vector<std::unique_ptr<SocketMessage>> returnEndpoints;

    auto request = std::move(generateFindBySchemaRequest(endpointType, otherValues));

    systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*request);

    for (const auto &rdh : resourceDiscoveryHubs) {

        std::unique_ptr<SocketMessage> reply;
        try {
            reply = sendRequest(rdh.first, *request);
            systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*reply);
        } catch (std::logic_error &e) {
            std::cerr << "Error getting component from " << rdh.first << "\n\t" << e.what() << std::endl;
            continue;
        }

        auto replyEndpoints = reply->getArray<std::unique_ptr<SocketMessage>>("endpoints");

        returnEndpoints.insert(
                returnEndpoints.end(),
                std::make_move_iterator(replyEndpoints.begin()),
                std::make_move_iterator(replyEndpoints.end())
        );
    }
    return returnEndpoints;
}


void ResourceDiscoveryConnEndpoint::createEndpointBySchema(const std::string &endpointType) {
    std::map<std::string, std::string> empty;
    std::vector<std::unique_ptr<SocketMessage>> validLocations = getComponentsBySchema(endpointType, empty);

    for (const auto &location: validLocations) {
        bool connection = false;
        for (const auto &url: location->getArray<std::string>("urls")) {
            try {
                if (location->hasMember("listenPort")) {
                    std::string dialUrl = url + ":" + std::to_string(location->getInteger("listenPort"));
                    component->createEndpointAndDial(endpointType, dialUrl);
                } else {
                    component->getBackgroundRequester().requestRemoteListenThenDial(url, location->getInteger("port"),
                                                                                    endpointType,
                                                                                    location->getString(
                                                                                            "endpointType"));
                }
                connection = true;
                break;
            } catch (std::logic_error &e) {
                continue;
            }
        }
        if (!connection) {
            std::cerr << "Wasn't able to connect to " << location->stringify() << std::endl;
        }
    }
}

void ResourceDiscoveryConnEndpoint::updateManifestWithHubs() {
    auto newManifest = component->getComponentManifest().getSocketMessageCopy();
    newManifest->addMember("urls", component->getAllAddresses());
    newManifest->addMember("port", component->getBackgroundPort());

    auto request = std::make_unique<SocketMessage>();
    request->addMember("request", RESOURCE_DISCOVERY_UPDATE_MANIFEST);
    request->addMoveObject("newManifest", std::move(newManifest));

    for (auto &locationIdPair : resourceDiscoveryHubs) {
        request->addMember("id", locationIdPair.second);
        systemSchemas.getSystemSchema(SystemSchemaName::updateRequest).validate(*request);
        try {
            auto reply = sendRequest(locationIdPair.first, *request);

        } catch (std::logic_error &e) {
            std::cerr << "Problem connecting to " << locationIdPair.first << "\n\t" << e.what() << std::endl;
        }

    }
}

void ResourceDiscoveryConnEndpoint::searchForResourceDiscoveryHubs() {
    auto addresses = component->getAllAddresses();
    bool found = false;
    if (component->getComponentConnectionType() == ConnectionType::IPC) {
        std::cerr << "Can't search for RDH's with IPC" << std::endl;
        return;
    }
    for (const auto &address: addresses) {
        if (found) { break; }

        std::string subnet = address.substr(0, address.rfind('.'));
        for (int i = 0; i <= 255; i++) {
            if (found) { break; }
            std::string dialAddress =
                    subnet + "." + std::to_string(i) + ":" + std::to_string(DEFAULT_BACKGROUND_LISTEN_PORT);
            try {
                std::vector<std::string> RDHs =
                        component->getBackgroundRequester().requestLocationsOfRDH(dialAddress);
                if (RDHs.empty()) { continue; }
                for (const std::string &url : RDHs) {
                    try {
                        registerWithHub(url);
                        found = true;
                        std::cout << "Found Resource Discovery Hub: " << url << std::endl;
                        break;
                    } catch (std::logic_error &e) {
                        continue;
                    }
                }
            } catch (std::logic_error &e) {
                continue;
            }
        }
    }
    if(!found){
        throw std::logic_error("Unable to find any Resource Discovery Hub");
    }
}

std::map<std::string, std::unique_ptr<ComponentRepresentation>>
ResourceDiscoveryConnEndpoint::getComponentsByProperties(std::map<std::string, std::string> &properties) {
    auto specialProperties = std::make_unique<SocketMessage>();
    for (auto &keyValuePair : properties) {
        specialProperties->addMember(keyValuePair.first, keyValuePair.second);
    }
    SocketMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_REQUEST_BY_PROPERTIES);
    request.addMoveObject("specialProperties", std::move(specialProperties));

    std::map<std::string, std::unique_ptr<ComponentRepresentation>> returnComponents;

    for (const auto &rdh : resourceDiscoveryHubs) {
        std::unique_ptr<SocketMessage> reply;
        try {
            reply = sendRequest(rdh.first, request);
        } catch (std::logic_error &e) {
            std::cerr << "Error getting component from " << rdh.first << "\n\t" << e.what() << std::endl;
            continue;
        }

        auto replyIDs = reply->getKeys();

        for (auto &compID:replyIDs) {
            if (compID == "error") { continue; }
            returnComponents.insert(std::make_pair(compID,
                                                   std::make_unique<ComponentRepresentation>(
                                                           reply->getMoveObject(compID).get(), systemSchemas)));
        }
    }

    return returnComponents;
}

void ResourceDiscoveryConnEndpoint::deRegisterFromHub(const std::string &rdhUrl) {
    SocketMessage request;
    request.addMember("request", RESOURCE_DISCOVERY_DEREGISTER_COMPONENT);
    try {
        request.addMember("id", resourceDiscoveryHubs.at(rdhUrl));
    } catch (std::out_of_range &e) {
        throw;
    }

    try {
        auto reply = sendRequest(rdhUrl, request);
        std::cout << "De-registered from " << rdhUrl << std::endl;
    } catch (std::logic_error &e) {
        std::cerr << "Error with de-register from " << rdhUrl << "\n" << e.what() << std::endl;
    }
    // Either way we don't want to contact that hub again
    resourceDiscoveryHubs.erase(rdhUrl);
}

void ResourceDiscoveryConnEndpoint::deRegisterFromAllHubs() {
    // Weird technique used so we can delete from resourceDiscoveryHubs as we go
    for (auto it = resourceDiscoveryHubs.cbegin(), next_it = it; it != resourceDiscoveryHubs.cend(); it = next_it) {
        // Looks ahead before deletion
        ++next_it;
        deRegisterFromHub(it->first);
    }
}

void ResourceDiscoveryConnEndpoint::addListenerPortForAllHubs(const std::string &endpointType, int port) {
    SocketMessage request;
    request.addMember("request",RESOURCE_DISCOVERY_NOTIFY_SOCKET_LISTEN);
    request.addMember("endpointType",endpointType);
    request.addMember("port",port);

    for (auto &locationIdPair : resourceDiscoveryHubs) {
        request.addMember("id", locationIdPair.second);
        try {
            auto reply = sendRequest(locationIdPair.first, request);

        } catch (std::logic_error &e) {
            std::cerr << "Problem connecting to " << locationIdPair.first << "\n\t" << e.what() << std::endl;
        }

    }
}
