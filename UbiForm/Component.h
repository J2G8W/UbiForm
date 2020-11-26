#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"

#include <iostream>
#include <memory>
#include <map>

#include "ComponentManifest.h"
#include "SocketMessage.h"
#include "endpoints/PairEndpoint.h"

class Component {
private:
    std::unique_ptr<ComponentManifest> componentManifest{nullptr};
    std::map<std::string, std::shared_ptr<DataReceiverEndpoint> > receiverEndpoints;
    std::map<std::string, std::shared_ptr<DataSenderEndpoint> > senderEndpoints;



public:
    Component() = default;

    void specifyManifest(FILE *jsonFP) { componentManifest = std::make_unique<ComponentManifest>(jsonFP); }

    void specifyManifest(const char *jsonString) {
        componentManifest = std::make_unique<ComponentManifest>(jsonString);
    }

    void createNewPairEndpoint(std::string type, std::string id);

    std::shared_ptr<DataReceiverEndpoint> getReceiverEndpoint(const std::string& id){
        try{
            return receiverEndpoints.at(id);
        }catch(std::out_of_range &e){
            throw;
        }
    };

    std::shared_ptr<DataSenderEndpoint> getSenderEndpoint(const std::string& id){
        try{
            return senderEndpoints.at(id);
        }catch(std::out_of_range &e){
            throw;
        }
    };


};


#endif //UBIFORM_COMPONENT_H
