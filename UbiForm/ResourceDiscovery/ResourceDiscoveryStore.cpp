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


    auto * returnMsg = new SocketMessage;
    if (request == ADDITION){
        rds.systemSchemas.at(RDMessaging::additionRequest)->validate(*sm);

        SocketMessage *manifest = sm->getObject("manifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest);
        delete manifest;

        std::string id = std::to_string(generator());

        auto p1 = std::make_pair(id, newCR);
        rds.componentById.insert(p1);

        returnMsg->addMember("id",id);

        rds.systemSchemas.at(RDMessaging::additionResponse)->validate(*returnMsg);

    }else if (request == REQUEST_BY_ID){
        rds.systemSchemas.at(RDMessaging::byIdRequest)->validate(*sm);

        std::string id = sm->getString("id");
        if (rds.componentById.count(id) > 0){
            std::string component =  rds.componentById.at(id)->stringify();
            SocketMessage componentObject(component.c_str());
            returnMsg->addMember("component",componentObject);
        }else{
            returnMsg->setNull("component");
        }

        rds.systemSchemas.at(RDMessaging::byIdResponse)->validate(*returnMsg);
    }else if (request == REQUEST_BY_SCHEMA){
        rds.systemSchemas.at(RDMessaging::bySchemaRequest)->validate(*sm);

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
        rds.systemSchemas.at(RDMessaging::bySchemaResponse)->validate(*returnMsg);
    }else if (request == REQUEST_COMPONENTS){
        rds.systemSchemas.at(RDMessaging::componentIdsRequest)->validate(*sm);
        std::vector<std::string> componentIds;
        for (auto & it : rds.componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
        rds.systemSchemas.at(RDMessaging::componentIdsResponse)->validate(*returnMsg);
    }
    return returnMsg;
}

ResourceDiscoveryStore::ResourceDiscoveryStore() {
    const char* files[8] = {"SystemSchemas/resource_discovery_addition_request.json",
                            "SystemSchemas/resource_discovery_addition_response.json",
                            "SystemSchemas/resource_discovery_by_id_request.json",
                            "SystemSchemas/resource_discovery_by_id_response.json",
                            "SystemSchemas/resource_discovery_by_schema_request.json",
                            "SystemSchemas/resource_discovery_by_schema_response.json",
                            "SystemSchemas/resource_discovery_component_ids_request.json",
                            "SystemSchemas/resource_discovery_component_ids_response.json"};

    for (int i =0; i < 8; i++){
        FILE* pFile = fopen(files[i], "r");
        if (pFile == NULL){
            std::cerr << "Error finding requisite file -" << files[i] << std::endl;
            exit(1);
        }
        std::unique_ptr<EndpointSchema> es = std::make_unique<EndpointSchema>(pFile);
        systemSchemas.insert(std::make_pair(static_cast<RDMessaging>(i), std::move(es)));
        fclose(pFile);
    }
}
