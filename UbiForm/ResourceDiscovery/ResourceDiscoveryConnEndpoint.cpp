#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"
#include "../../include/UbiForm/Component.h"
#include <nng/protocol/reqrep0/req.h>
#include <nng/supplemental/util/platform.h>


void ResourceDiscoveryConnEndpoint::handleAsyncReceive(SocketMessage *sm, void *data) {
    if(sm->getBoolean("error")){
        if (sm->hasMember("errorMsg")) {
            std::cerr << "Remote error with request: " + sm->getString("errorMsg") << std::endl;
        } else {
            std::cerr << "Remote error with request, no error message received" << std::endl;
        }
    }
}

std::unique_ptr<SocketMessage>
ResourceDiscoveryConnEndpoint::sendRequest(const std::string &url, SocketMessage &request, bool waitForResponse) {

    if(resourceDiscoveryEndpoints.count(url) == 0){
        throw AccessError("No endpoint for " + url + " open yet");
    }
    resourceDiscoveryEndpoints.at(url)->sendMessage(request);
    if(waitForResponse) {
        auto reply = resourceDiscoveryEndpoints.at(url)->receiveMessage();
        if (reply->getBoolean("error")) {
            if (reply->hasMember("errorMsg")) {
                throw RemoteError("Error with request: " + reply->getString("errorMsg"), url);
            } else {
                throw RemoteError("Error with request, no error message", url);
            }
        }
        return reply;
    }else{
        resourceDiscoveryEndpoints.at(url)->asyncReceiveMessage(handleAsyncReceive, nullptr);
        return nullptr;
    }
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
    std::unique_ptr<RequestEndpoint> newEndpoint =
            std::make_unique<RequestEndpoint>(
                    systemSchemas.getSystemSchema(SystemSchemaName::generalRDResponse).getInternalSchema(),
                    systemSchemas.getSystemSchema(SystemSchemaName::generalRDRequest).getInternalSchema(),
                    "Resource Discovery Connection", "RDC - " + url);
    resourceDiscoveryEndpoints[url] = std::move(newEndpoint);
    try {
        resourceDiscoveryEndpoints.at(url)->dialConnection(url.c_str());

        reply = sendRequest(url, *request, true);
        systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
        std::cout << "Registered successfully with: " << url << " id is " << reply->getString("newID") << std::endl;
    } catch (std::logic_error &e) {
        resourceDiscoveryEndpoints.erase(url);
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
        reply = sendRequest(url, request, true);
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
        reply = sendRequest(url, request, true);
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
            reply = sendRequest(rdh.first, *request, true);
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
                // First try the listenPort, if that fails then request remote
                if (location->hasMember("listenPort")) {
                    try {
                        std::string dialUrl = url + ":" + std::to_string(location->getInteger("listenPort"));
                        component->createEndpointAndDial(endpointType, dialUrl);
                        connection = true;
                        break;
                    } catch (std::logic_error &e) {
                        // PURPOSELY EMPTY
                    }
                }
                component->getBackgroundRequester().requestRemoteListenThenDial(url, location->getInteger("port"),
                                                                                endpointType,
                                                                                location->getString("endpointType"));
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
            sendRequest(locationIdPair.first, *request, false);

        } catch (std::logic_error &e) {
            std::cerr << "Problem connecting to " << locationIdPair.first << "\n\t" << e.what() << std::endl;
        }

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
            reply = sendRequest(rdh.first, request, true);
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
        sendRequest(rdhUrl, request, false);
        std::cout << "De-registered from " << rdhUrl << std::endl;
    } catch (std::logic_error &e) {
        std::cerr << "Error with de-register from " << rdhUrl << "\n" << e.what() << std::endl;
    }
    // Either way we don't want to contact that hub again
    resourceDiscoveryHubs.erase(rdhUrl);
    resourceDiscoveryEndpoints.erase(rdhUrl);
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
            sendRequest(locationIdPair.first, request, false);

        } catch (std::logic_error &e) {
            std::cerr << "Problem connecting to " << locationIdPair.first << "\n\t" << e.what() << std::endl;
        }

    }
}

void ResourceDiscoveryConnEndpoint::checkLivenessOfHubs() {
    auto it = resourceDiscoveryHubs.begin();
    while(it != resourceDiscoveryHubs.end()){
        try{
            getComponentIdsFromHub(it->first);
        } catch (AccessError &e) {
            // Ignored - endpoint no longer valid
        } catch (RemoteError &e) {
            // Ignored - hub is valid, but had an issue with
        } catch (std::logic_error &e){
            it = resourceDiscoveryHubs.erase(it);
            resourceDiscoveryEndpoints.erase(it->first);
            continue;
        }
        it++;
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
        std::vector<std::thread> threads;
        // TODO - Memory leak of booleans
        std::vector<bool*> alive;

        for (int i = 0; i <= 255; i++) {
            std::string dialAddress =
                    subnet + "." + std::to_string(i) + ":" + std::to_string(DEFAULT_RESOURCE_DISCOVERY_PORT);
            bool* t = new bool(false);
            alive.push_back(t);
            threads.emplace_back(asyncCheckHubLive,dialAddress, alive.at(i), this);
        }

        // Assumption here is that we will find the thing pretty quickly
        nng_msleep(1000);
        for (int i = 0; i<= 255; i++){
            threads.at(i).detach();
            if(*(alive.at(i))) {
                std::string dialAddress =
                        subnet + "." + std::to_string(i) + ":" + std::to_string(DEFAULT_RESOURCE_DISCOVERY_PORT);
                try {
                    registerWithHub(dialAddress);
                    found = true;
                    std::cout << "Found Resource Discovery Hub: " << dialAddress << std::endl;
                } catch (std::logic_error &e) {
                    continue;
                }
            }

        }
    }
    std::cout << "End" << std::endl;
    if(!found){
        throw std::logic_error("Unable to find any Resource Discovery Hub");
    }
}

void ResourceDiscoveryConnEndpoint::asyncCheckHubLive(const std::string &url, bool *returnVal,ResourceDiscoveryConnEndpoint* rdc) {
    RequestEndpoint newEndpoint (
            rdc->systemSchemas.getSystemSchema(SystemSchemaName::generalRDResponse).getInternalSchema(),
            rdc->systemSchemas.getSystemSchema(SystemSchemaName::generalRDRequest).getInternalSchema(),
            "Resource Discovery Connection", "Test RDC - " + url);
    try{
        std::cout << url << std::endl;
        newEndpoint.dialConnection(url.c_str());
        SocketMessage sm;
        sm.addMember("request",RESOURCE_DISCOVERY_REQUEST_ALIVE);
        newEndpoint.sendMessage(sm);
        newEndpoint.setReceiveTimeout(300);
        newEndpoint.receiveMessage();
        *returnVal = true;
    } catch (NngError &e) {
        *returnVal = false;
    }
}

