#ifndef UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
#define UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H


#include <nng/nng.h>
#include <string>
#include <thread>
#include <map>

#include "ComponentRepresentation.h"
#include "ResourceDiscoveryStore.h"
#include "../Endpoints/ReplyEndpoint.h"

class ResourceDiscoveryHubEndpoint {
private:
    std::thread rdThread;

    ReplyEndpoint replyEndpoint;
    ResourceDiscoveryStore rdStore;

    std::string listenAddress;

    static void rdBackground(ResourceDiscoveryHubEndpoint *);

public:
    explicit ResourceDiscoveryHubEndpoint(SystemSchemas &ss) : rdStore(ss),
     replyEndpoint(std::make_shared<EndpointSchema>(), std::make_shared<EndpointSchema>(), "ResourceDiscoveryHub") {}
    void startResourceDiscover(const std::string &baseAddress, int port);

    std::string getListenAddress(){
        if (listenAddress == ""){
            throw std::logic_error("Resource Discovery Hub has not started yet");
        }else{
            return listenAddress;
        }
    }

    ~ResourceDiscoveryHubEndpoint();

};



#endif //UBIFORM_RESOURCEDISCOVERYHUBENDPOINT_H
