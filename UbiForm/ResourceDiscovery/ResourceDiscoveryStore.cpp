#include <random>
#include <chrono>
#include "ResourceDiscoveryStore.h"

SocketMessage *ResourceDiscoveryStore::generateRDResponse(SocketMessage *sm, ResourceDiscoveryStore &rds) {
    std::string request;
    try{
        request= sm->getString("request");
    }catch(AccessError &e){
        throw ValidationError("Message has no request field");
    }

    std::cout << "Resource Discovery Request - " << request << std::endl;

    // Use a unique_ptr so when exceptions thrown it auto deletes
    std::unique_ptr<SocketMessage> returnMsg = std::make_unique<SocketMessage>();
    if (request == ADDITION){

        rds.systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*sm);

        // We copy the manifest object when creating ComponentRepresentation anyway
        auto manifest = sm->getMoveObject("manifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest.get(), rds.systemSchemas);

        std::string id = std::to_string(rds.generator());

        auto p1 = std::make_pair(id, newCR);
        rds.componentById.insert(p1);

        returnMsg->addMember("newID",id);

        rds.systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*returnMsg);

    }else if (request == REQUEST_BY_ID){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(*sm);

        std::string id = sm->getString("id");
        if (rds.componentById.count(id) > 0){
            std::string component =  rds.componentById.at(id)->stringify();
            auto componentObject = std::make_unique<SocketMessage>(component.c_str());
            returnMsg->moveMember("component",std::move(componentObject));
        }else{
            returnMsg->setNull("component");
        }

        rds.systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*returnMsg);
    }else if (request == REQUEST_BY_SCHEMA){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*sm);

        auto schemaRequest = sm->getMoveObject("schema");
        bool receiveData = sm->getBoolean("dataReceiverEndpoint");
        std::vector<SocketMessage *> returnEndpoints;

        for (const auto &componentRep : rds.componentById){
            std::vector<std::string> endpointIds = componentRep.second->findEquals(receiveData, *schemaRequest);
            for (const auto &id: endpointIds){
                auto * endpoint = new SocketMessage;

                endpoint->addMember("componentId", componentRep.first);
                endpoint->addMember("urls",componentRep.second->getAllUrls());
                endpoint->addMember("port",componentRep.second->getPort());
                endpoint->addMember("endpointType", id);
                returnEndpoints.emplace_back(endpoint);
            }
        }
        returnMsg->addMember("endpoints", returnEndpoints);
        for (auto rs: returnEndpoints){
            delete rs;
        }

        rds.systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*returnMsg);
    }else if (request == REQUEST_COMPONENTS){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(*sm);
        std::vector<std::string> componentIds;
        for (auto & it : rds.componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
        rds.systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*returnMsg);
    }else if (request == UPDATE){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::updateRequest).validate(*sm);
        auto manifest = sm->getMoveObject("newManifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest.get(), rds.systemSchemas);

        std::string id = sm->getString("id");

        rds.componentById[id] = newCR;

        returnMsg->addMember("newID",id);

        rds.systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*returnMsg);

    }
    return returnMsg.release();
}

ResourceDiscoveryStore::ResourceDiscoveryStore(SystemSchemas & ss) : systemSchemas(ss), generator() {
    long randomSeed = std::chrono::system_clock::now().time_since_epoch().count();
    generator.seed(randomSeed);
}
