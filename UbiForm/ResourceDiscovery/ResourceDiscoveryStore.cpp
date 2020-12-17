#include <random>
#include "ResourceDiscoveryStore.h"

std::minstd_rand0 ResourceDiscoveryStore::generator(0);

SocketMessage *ResourceDiscoveryStore::generateRDResponse(SocketMessage *sm, ResourceDiscoveryStore &rds) {
    std::string request = sm->getString("request");
    std::cout << request << std::endl;
    auto * returnMsg = new SocketMessage;
    if (request == ADDITION){
        SocketMessage *manifest = sm->getObject("manifest");

        auto newCR = std::make_shared<ComponentRepresentation>(manifest);
        std::string id = std::to_string(generator());
        auto p1 = std::make_pair(id, newCR);
        rds.componentById.insert(p1);
        returnMsg->addMember("id",id);
        delete manifest;
    }else if (request == REQUEST_BY_ID){
        std::string id = sm->getString("id");
        if (rds.componentById.count(id) > 0){
            std::string component =  rds.componentById.at(id)->stringify();
            SocketMessage componentObject(component.c_str());
            returnMsg->addMember("component",componentObject);
        }else{
            returnMsg->setNull("component");
        }
    }else if (request == REQUEST_BY_SCHEMA){
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
                endpoint->addMember("socketType", "TODO");
                returnEndpoints.emplace_back(endpoint);
            }
        }
        returnMsg->addMember("endpoints", returnEndpoints);
        for (auto rs: returnEndpoints){
            delete rs;
        }
        delete schemaRequest;

    }else if (request == REQUEST_COMPONENTS){
        std::vector<std::string> componentIds;
        for (auto & it : rds.componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
    }
    return returnMsg;
}
