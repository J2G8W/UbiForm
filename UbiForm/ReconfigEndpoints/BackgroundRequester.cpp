// Created by julian on 05/01/2021.
//

#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "BackgroundRequester.h"
#include "BackgroundListener.h"
#include "../Component.h"

// THIS IS OUR COMPONENT MAKING REQUESTS FOR A NEW CONNECTION
void BackgroundRequester::requestAndCreateConnection(const std::string &baseAddress, int port,
                                                     const std::string &localEndpointType,
                                                     const std::string &remoteEndpointType) {

    std::string requestSocketType = component->getComponentManifest().getSocketType(localEndpointType);
    SocketMessage sm;
    if (requestSocketType == SUBSCRIBER){
        sm.addMember("socketType",PUBLISHER);
    }else{
        sm.addMember("socketType",requestSocketType);
    }

    sm.addMember("endpointType",remoteEndpointType);
    sm.addMember("requestType", REQ_CONN);

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);

    std::string dialAddress = baseAddress + ":"  + std::to_string(port);
    try{
        requestEndpoint.dialConnection(dialAddress.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if (reply->getBoolean("error")){
            throw std::logic_error("Error request and create connection: " + reply->getString("errorMsg"));
        }else{
            int newPort = reply->getInteger("port");
            component->createEndpointAndDial(requestSocketType, localEndpointType, baseAddress + ":" + std::to_string(newPort));
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl) {
    SocketMessage sm;
    sm.addMember("requestType",ADD_RDH);
    sm.addMember("url",rdhUrl);
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if(reply->getBoolean("error")){
            throw std::logic_error("Error in request add RDH: " + reply->getString("errorMsg"));
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::tellToRequestAndCreateConnection(const std::string &requesterAddress,
                                                           const std::string &requesterEndpointType,
                                                           const std::string &remoteEndpointType,
                                                           const std::string &remoteAddress, int newPort) {
    SocketMessage sm;
    sm.addMember("requestType", TELL_REQ_CONN);
    sm.addMember("reqEndpointType", requesterEndpointType);
    sm.addMember("remoteEndpointType",remoteEndpointType);
    sm.addMember("remoteAddress", remoteAddress);
    sm.addMember("port",newPort);
    try{
        requestEndpoint.dialConnection(requesterAddress.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if (reply->getBoolean("error")){
            throw std::logic_error("Error with remote request and create connection: " +reply->getString("errorMsg"));
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::requestChangeEndpoint(const std::string &componentAddress, SocketType socketType,
                                                const std::string &endpointType, EndpointSchema *receiverSchema,
                                                EndpointSchema *sendSchema) {
    SocketMessage sm;
    sm.addMember("requestType", CHANGE_ENDPOINT_SCHEMA);
    sm.addMember("endpointType", endpointType);
    if (receiverSchema == nullptr){sm.setNull("receiveSchema");}
    else{
        auto schemaObj = std::unique_ptr<SocketMessage>(receiverSchema->getSchemaObject());
        sm.moveMember("receiveSchema", std::move(schemaObj));
    }
    if (sendSchema == nullptr){sm.setNull("sendSchema");}
    else{
        auto schemaObj = std::unique_ptr<SocketMessage>(sendSchema->getSchemaObject());
        sm.moveMember("sendSchema", std::move(schemaObj));
    }
    sm.addMember("socketType",socketType);

    try{
        requestEndpoint.dialConnection(componentAddress.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if (reply->getBoolean("error")){
            throw std::logic_error("Error with request add endpoint schema: " + reply->getString("errorMsg"));
        }
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

int BackgroundRequester::requestCreateRDH(const std::string &componentUrl) {
    SocketMessage sm;
    sm.addMember("requestType",CREATE_RDH);
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if(!reply->getBoolean("error")){
            return reply->getInteger("port");
        }else{
            throw std::logic_error("Error with request to create RDH: " + reply->getString("errorMsg"));
        }
    }catch(std::logic_error &e){
        std::cerr << e.what() << std::endl;
        // Should return empty or throw exception?
        throw;
    }
}

// Is necceassry?
void BackgroundRequester::requestToCreateAndDial(const std::string &componentUrl, const std::string &socketType,
                                                 const std::string &endpointType, const std::string &remoteUrl) {
}

// Return empty if error reply
std::vector<std::string> BackgroundRequester::requestLocationsOfRDH(const std::string &componentUrl) {
    SocketMessage sm;
    sm.addMember("requestType", LOCATIONS_OF_RDH);
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if(reply->getBoolean("error")){
            throw std::logic_error("Error with request to update component manifest " + reply->getString("errorMsg"));
        }
        std::vector<std::string> locations = reply->getArray<std::string>("locations");
        return locations;
    }catch(std::logic_error &e){
        std::cerr << e.what() << std::endl;
        return std::vector<std::string>();
    }
}

void BackgroundRequester::requestCloseSocketOfType(const std::string &componentUrl, const std::string& endpointType) {
    SocketMessage sm;
    sm.addMember("requestType",CLOSE_SOCKETS);
    sm.addMember("endpointType",endpointType);
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if(reply->getBoolean("error")){
            throw std::logic_error("Error with request to close sockets " + reply->getString("errorMsg"));
        }
    }catch(std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}

void BackgroundRequester::requestUpdateComponentManifest(const std::string &componentUrl, ComponentManifest &newManifest) {
    SocketMessage sm;
    sm.addMember("requestType",CHANGE_MANIFEST);
    auto compRep = newManifest.getComponentRepresentation();
    sm.moveMember("newManifest",std::move(compRep));
    try{
        requestEndpoint.dialConnection(componentUrl.c_str());
        requestEndpoint.sendMessage(sm);
        auto reply = requestEndpoint.receiveMessage();
        if(reply->getBoolean("error")){
            throw std::logic_error("Error with request to update component manifest " + reply->getString("errorMsg"));
        }
    }catch(std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}
