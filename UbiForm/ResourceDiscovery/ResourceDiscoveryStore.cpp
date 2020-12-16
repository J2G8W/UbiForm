//
// Created by julian on 16/12/2020.
//

#include <random>
#include "ResourceDiscoveryStore.h"

SocketMessage *ResourceDiscoveryStore::generateRDResponse(SocketMessage *sm, ResourceDiscoveryStore &rds) {
    std::string request = sm->getString("request");
    auto * returnMsg = new SocketMessage;
    if (request == ADDITION){
        SocketMessage *manifest = sm->getObject("manifest");
        auto newCR = std::make_shared<ComponentRepresentation>(manifest);
        std::minstd_rand0 generator (0);
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

    }else if (request == REQUEST_COMPONENTS){
        std::vector<std::string> componentIds;
        for (auto & it : rds.componentById){
            componentIds.push_back(it.first);
        }
        returnMsg->addMember("components", componentIds);
    }
    return returnMsg;
}
