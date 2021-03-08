#include <random>
#include <chrono>
#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryStore.h"

std::unique_ptr<SocketMessage> ResourceDiscoveryStore::generateRDResponse(SocketMessage *sm) {
    std::string request;
    try {
        request = sm->getString("request");
    } catch (AccessError &e) {
        throw ValidationError("Message has no request field");
    }

    //std::cout << "Resource Discovery Request - " << request << std::endl;

    // Use a unique_ptr so when exceptions thrown it auto deletes
    std::unique_ptr<SocketMessage> returnMsg = std::make_unique<SocketMessage>();
    if (request == RESOURCE_DISCOVERY_ADD_COMPONENT) {

        systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*sm);

        // We copy the manifest object when creating ComponentRepresentation anyway
        auto manifest = sm->getMoveObject("manifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest.get(), systemSchemas);

        std::string id = std::to_string(generator());

        auto p1 = std::make_pair(id, newCR);
        componentById.insert(p1);

        returnMsg->addMember("newID", id);

        systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*returnMsg);

    } else if (request == RESOURCE_DISCOVERY_REQUEST_BY_ID) {
        systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(*sm);

        std::string id = sm->getString("id");
        if (componentById.count(id) > 0) {
            std::string component = componentById.at(id)->stringify();
            auto componentObject = std::make_unique<SocketMessage>(component.c_str());
            returnMsg->addMoveObject("component", std::move(componentObject));
        } else {
            returnMsg->setNull("component");
        }

        systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*returnMsg);
    } else if (request == RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA) {
        systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*sm);

        auto schemaRequest = sm->getMoveObject("schema");
        bool receiveData = sm->getBoolean("dataReceiverEndpoint");
        std::vector<SocketMessage *> returnEndpoints;

        auto specialProperties = sm->getMoveObject("specialProperties");
        std::map<std::string, std::string> propertiesMap;
        for (const auto &key:specialProperties->getKeys()) {
            propertiesMap.insert(std::make_pair(key, specialProperties->getString(key)));
        }

        for (const auto &componentRep : componentById) {
            bool validComponent = true;
            for (auto &pair : propertiesMap) {
                if (!(componentRep.second->hasProperty(pair.first) &&
                      componentRep.second->getProperty(pair.first) == pair.second)) {
                    validComponent = false;
                    break;
                }
            }
            if (!validComponent) { continue; }

            std::vector<std::string> endpointIds = componentRep.second->findEquals(receiveData, *schemaRequest);
            for (const auto &id: endpointIds) {
                auto *endpoint = new SocketMessage;
                endpoint->addMember("componentId", componentRep.first);
                endpoint->addMember("urls", componentRep.second->getAllUrls());
                endpoint->addMember("port", componentRep.second->getPort());
                endpoint->addMember("endpointType", id);
                if (componentRep.second->hasListenPort(id)) {
                    endpoint->addMember("listenPort", componentRep.second->getListenPort(id));
                }
                returnEndpoints.emplace_back(endpoint);
            }
        }
        returnMsg->addMember("endpoints", returnEndpoints);
        for (auto rs: returnEndpoints) {
            delete rs;
        }

        systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*returnMsg);
    } else if (request == RESOURCE_DISCOVERY_REQUEST_COMPONENTS) {
        systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(*sm);
        std::vector<std::string> componentIds;
        for (auto &it : componentById) {
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
        systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*returnMsg);
    } else if (request == RESOURCE_DISCOVERY_UPDATE_MANIFEST) {
        systemSchemas.getSystemSchema(SystemSchemaName::updateRequest).validate(*sm);
        auto manifest = sm->getMoveObject("newManifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest.get(), systemSchemas);

        std::string id = sm->getString("id");

        componentById[id] = newCR;

    } else if (request == RESOURCE_DISCOVERY_REQUEST_BY_PROPERTIES) {
        // Not validated, will return access error on failure
        auto specialProperties = sm->getMoveObject("specialProperties");
        auto specialKeys = specialProperties->getKeys();

        for (const auto &componentRep : componentById) {
            bool validComponent = true;
            for (auto &key : specialKeys) {
                if (!(componentRep.second->hasProperty(key) &&
                      componentRep.second->getProperty(key) == specialProperties->getString(key))) {
                    validComponent = false;
                    break;
                }
            }
            if (validComponent) {
                returnMsg->addMoveObject(componentRep.first, componentRep.second->getSocketMessageCopy());
            }
        }
    } else if (request == RESOURCE_DISCOVERY_DEREGISTER_COMPONENT) {
        std::string id = sm->getString("id");
        if (componentById.count(id) == 1) {
            componentById.erase(id);
        }
    } else if (request == RESOURCE_DISCOVERY_NOTIFY_SOCKET_LISTEN){
        componentById.at(sm->getString("id"))->addListenPort(
                sm->getString("endpointType"),sm->getInteger("port"));
    } else if (request == RESOURCE_DISCOVERY_REQUEST_ALIVE){
        returnMsg->addMember("live",true);
    } else {
        throw std::logic_error("Error with request: "+ request +"\nDid not match expected");
    }
    return returnMsg;
}

ResourceDiscoveryStore::ResourceDiscoveryStore(SystemSchemas &ss) : systemSchemas(ss), generator() {
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);
}

std::vector<std::shared_ptr<ComponentRepresentation>> ResourceDiscoveryStore::getConnections() {
    std::vector<std::shared_ptr<ComponentRepresentation>> connections;
    for(const auto& pair : componentById){
        connections.emplace_back(pair.second);
    }
    return connections;
}