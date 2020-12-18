#include <random>
#include "ResourceDiscoveryStore.h"

std::minstd_rand0 ResourceDiscoveryStore::generator(0);

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


        SocketMessage *manifest = sm->getObject("manifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest, rds.systemSchemas);
        delete manifest;

        std::string id = std::to_string(generator());

        auto p1 = std::make_pair(id, newCR);
        rds.componentById.insert(p1);

        returnMsg->addMember("id",id);

        rds.systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*returnMsg);

    }else if (request == REQUEST_BY_ID){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(*sm);

        std::string id = sm->getString("id");
        if (rds.componentById.count(id) > 0){
            std::string component =  rds.componentById.at(id)->stringify();
            SocketMessage componentObject(component.c_str());
            returnMsg->addMember("component",componentObject);
        }else{
            returnMsg->setNull("component");
        }

        rds.systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*returnMsg);
    }else if (request == REQUEST_BY_SCHEMA){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*sm);

        SocketMessage * schemaRequest = sm->getObject("schema");
        bool receiveData = sm->getBoolean("dataReceiverEndpoint");
        std::vector<SocketMessage *> returnEndpoints;

        for (const auto &componentRep : rds.componentById){
            std::vector<std::string> endpointIds = componentRep.second->findEquals(receiveData, *schemaRequest);
            for (const auto &id: endpointIds){
                auto * endpoint = new SocketMessage;

                endpoint->addMember("componentId", componentRep.first);
                endpoint->addMember("url",componentRep.second->getUrl());
                endpoint->addMember("endpointType", id);
                endpoint->addMember("socketType", "pair");
                returnEndpoints.emplace_back(endpoint);
            }
        }
        returnMsg->addMember("endpoints", returnEndpoints);
        for (auto rs: returnEndpoints){
            delete rs;
        }
        delete schemaRequest;
        rds.systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*returnMsg);
    }else if (request == REQUEST_COMPONENTS){
        rds.systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(*sm);
        std::vector<std::string> componentIds;
        for (auto & it : rds.componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
        rds.systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*returnMsg);
    }
    return returnMsg.release();
}

ResourceDiscoveryStore::ResourceDiscoveryStore(SystemSchemas & ss) : systemSchemas(ss) {
}
