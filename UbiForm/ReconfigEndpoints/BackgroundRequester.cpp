// Created by julian on 05/01/2021.
//

#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "BackgroundRequester.h"
#include "BackgroundListener.h"
#include "../Component.h"

// THIS IS OUR COMPONENT MAKING REQUESTS FOR A NEW CONNECTION
void BackgroundRequester::requestAndCreateConnection(const std::string& localEndpointType, const std::string &connectionComponentAddress,
                                           const std::string &remoteEndpointType) {
    std::string requestSocketType = component->getComponentManifest()->getSocketType(localEndpointType);
    SocketMessage sm;
    if (requestSocketType == SUBSCRIBER){
        sm.addMember("socketType",PUBLISHER);
    }else{
        sm.addMember("socketType",requestSocketType);
    }

    sm.addMember("endpointType",remoteEndpointType);
    sm.addMember("requestType", REQ_CONN);

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);

    std::string url;
    try{
        requestEndpoint.dialConnection(connectionComponentAddress.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if (reply->isNull("url")){
            throw std::logic_error("No valid endpoint of: " + remoteEndpointType);
        }else{
            url = reply->getString("url");
            component->createEndpointAndDial(requestSocketType, localEndpointType, url);
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::requestAddRDH(const std::string& rdhUrl, const std::string& componentUrl){
    SocketMessage sm;
    sm.addMember("requestType",ADD_RDH);
    sm.addMember("url",rdhUrl);
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        requestEndpoint.receiveMessage();
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::tellToRequestAndCreateConnection(const std::string& requesterEndpointType, const std::string &requesterAddress,
                                                           const std::string& remoteEndpointType, const std::string &remoteAddress){
    SocketMessage sm;
    sm.addMember("requestType", TELL_REQ_CONN);
    sm.addMember("reqEndpointType", requesterEndpointType);
    sm.addMember("remoteEndpointType",remoteEndpointType);
    sm.addMember("remoteAddress", remoteAddress);
    try{
        requestEndpoint.dialConnection(requesterAddress.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if (reply->getBoolean("error")){
            throw std::logic_error(reply->getString("errorMsg"));
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}