#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "BackgroundRequester.h"
#include "BackgroundListener.h"
#include "../Component.h"

std::unique_ptr<SocketMessage> BackgroundRequester::sendRequest(const std::string &url, SocketMessage &request) {
    requestEndpoint.dialConnection(url.c_str());
    requestEndpoint.sendMessage(request);
    auto reply = requestEndpoint.receiveMessage();
    if(reply->getBoolean("error")){
        if(reply->hasMember("errorMsg")){
            throw RemoteError(reply->getString("errorMsg"), url);
        }else{
            throw RemoteError("No Error message",url);
        }
    }
    return reply;
}


void BackgroundRequester::requestAndCreateConnection(const std::string &baseAddress, int port,
                                                     const std::string &localEndpointType,
                                                     const std::string &remoteEndpointType) {

    std::string localSocketType = component->getComponentManifest().getSocketType(localEndpointType);
    SocketMessage sm;
    if (localSocketType == SUBSCRIBER){
        sm.addMember("socketType",PUBLISHER);
    }else if(localSocketType == REQUEST){
        sm.addMember("socketType", REPLY);
    }
    else{
        sm.addMember("socketType", localSocketType);
    }

    sm.addMember("endpointType",remoteEndpointType);
    sm.addMember("requestType", BACKGROUND_REQUEST_CONNECTION);

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);

    std::string dialAddress = baseAddress + ":"  + std::to_string(port);

    auto reply = sendRequest(dialAddress,sm);

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(*reply);
    int newPort = reply->getInteger("port");
    component->createEndpointAndDial(localSocketType, localEndpointType, baseAddress + ":" + std::to_string(newPort));
}

void BackgroundRequester::requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_ADD_RDH);
    sm.addMember("url",rdhUrl);

    sendRequest(componentUrl,sm);
}

void BackgroundRequester::tellToRequestAndCreateConnection(const std::string &requesterAddress,
                                                           const std::string &requesterEndpointType,
                                                           const std::string &remoteEndpointType,
                                                           const std::string &remoteAddress, int newPort) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_TELL_TO_REQUEST_CONNECTION);
    sm.addMember("reqEndpointType", requesterEndpointType);
    sm.addMember("remoteEndpointType",remoteEndpointType);
    sm.addMember("remoteAddress", remoteAddress);
    sm.addMember("port",newPort);

    sendRequest(requesterAddress,sm);

}

void BackgroundRequester::requestChangeEndpoint(const std::string &componentAddress, SocketType socketType,
                                                const std::string &endpointType, EndpointSchema *receiverSchema,
                                                EndpointSchema *sendSchema) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_CHANGE_ENDPOINT_SCHEMA);
    sm.addMember("endpointType", endpointType);
    if (receiverSchema == nullptr){sm.setNull("receiveSchema");}
    else{
        auto schemaObj = std::unique_ptr<SocketMessage>(receiverSchema->getSchemaObject());
        sm.addMoveObject("receiveSchema", std::move(schemaObj));
    }
    if (sendSchema == nullptr){sm.setNull("sendSchema");}
    else{
        auto schemaObj = std::unique_ptr<SocketMessage>(sendSchema->getSchemaObject());
        sm.addMoveObject("sendSchema", std::move(schemaObj));
    }
    sm.addMember("socketType",socketType);


    sendRequest(componentAddress,sm);
}

int BackgroundRequester::requestCreateRDH(const std::string &componentUrl) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_CREATE_RDH);

    auto reply = sendRequest(componentUrl,sm);
    try {
        return reply->getInteger("port");
    }catch(AccessError &e){
        throw ValidationError("No port number in returned message");
    }
}

// Is necceassry?
void BackgroundRequester::requestToCreateAndDial(const std::string &componentUrl, const std::string &socketType,
                                                 const std::string &endpointType, const std::string &remoteUrl) {
}

// Return empty if error reply
std::vector<std::string> BackgroundRequester::requestLocationsOfRDH(const std::string &componentUrl) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_GET_LOCATIONS_OF_RDH);

    auto reply = sendRequest(componentUrl,sm);
    std::vector<std::string> locations;;
    try {
        locations = reply->getArray<std::string>("locations");
    }catch(AccessError &e){
        throw ValidationError("Returned message had no appropriate \"locations\"");
    }
    return locations;

}

void BackgroundRequester::requestCloseSocketOfType(const std::string &componentUrl, const std::string& endpointType) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_CLOSE_SOCKETS);
    sm.addMember("endpointType",endpointType);

    sendRequest(componentUrl,sm);

}

void BackgroundRequester::requestUpdateComponentManifest(const std::string &componentUrl, ComponentManifest &newManifest) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_CHANGE_MANIFEST);
    auto compRep = newManifest.getSocketMessageCopy();
    sm.addMoveObject("newManifest", std::move(compRep));

    sendRequest(componentUrl, sm);
}

void BackgroundRequester::requestCloseRDH(const std::string &componentUrl) {
    SocketMessage sm;
    sm.addMember("requestType", BACKGROUND_CLOSE_RDH);

    sendRequest(componentUrl, sm);

}
