#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "../../include/UbiForm/ReconfigurationEndpoints/BackgroundRequester.h"
#include "../../include/UbiForm/ReconfigurationEndpoints/BackgroundListener.h"
#include "../../include/UbiForm/Component.h"

std::unique_ptr<EndpointMessage> BackgroundRequester::sendRequest(const std::string &url, EndpointMessage &request) {
    requestEndpoint.dialConnection(url);
    requestEndpoint.setSendTimeout(500);
    requestEndpoint.setReceiveTimeout(500);
    requestEndpoint.sendMessage(request);
    auto reply = requestEndpoint.receiveMessage();
    if (reply->getBoolean("error")) {
        if (reply->hasMember("errorMsg")) {
            throw RemoteError(reply->getString("errorMsg"), url);
        } else {
            throw RemoteError("No Error message", url);
        }
    }
    return reply;
}


void BackgroundRequester::requestRemoteListenThenDial(const std::string &locationOfRemote, int remotePort,
                                                      const std::string &localEndpointType,
                                                      const std::string &remoteEndpointType) {
    int port = requestToCreateAndListen(locationOfRemote + ":" + std::to_string(remotePort), remoteEndpointType);
    component->createEndpointAndDial(localEndpointType, locationOfRemote + ":" + std::to_string(port));
}

int
BackgroundRequester::requestToCreateAndListen(const std::string &componentAddress, const std::string &endpointType) {
    EndpointMessage sm;
    sm.addMember("endpointType", endpointType);
    sm.addMember("requestType", BACKGROUND_CREATE_AND_LISTEN);

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);
    auto reply = sendRequest(componentAddress, sm);
    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationResponse).validate(*reply);
    return reply->getInteger("port");
}


void BackgroundRequester::localListenThenRequestRemoteDial(const std::string &componentAddress,
                                                           const std::string &localEndpointType,
                                                           const std::string &remoteEndpointType) {
    int listeningPort = component->createEndpointAndListen(localEndpointType);
    std::vector<std::string> selfLocations;
    for (const auto &addr: component->getAllAddresses()) {
        selfLocations.emplace_back(addr + ":" + std::to_string(listeningPort));
    }
    requestToCreateAndDial(componentAddress, remoteEndpointType, selfLocations);
}

void BackgroundRequester::requestToCreateAndDial(const std::string &componentUrl, const std::string &endpointType,
                                                 const std::vector<std::string> &remoteUrls) {
    EndpointMessage sm;

    sm.addMember("endpointType", endpointType);
    sm.addMember("requestType", BACKGROUND_CREATE_AND_DIAL);
    sm.addMember("dialUrls", remoteUrls);

    auto reply = sendRequest(componentUrl, sm);
}


void BackgroundRequester::requestAddRDH(const std::string &componentUrl, const std::string &rdhUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_ADD_RDH);
    sm.addMember("url", rdhUrl);

    sendRequest(componentUrl, sm);
}
void BackgroundRequester::requestRemoveRDH(const std::string &componentUrl, const std::string &rdhUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_REMOVE_RDH);
    sm.addMember("url",rdhUrl);
    sendRequest(componentUrl, sm);
}

void BackgroundRequester::request3rdPartyRemoteListenThenDial(const std::string &requesterAddress,
                                                              const std::string &requesterEndpointType,
                                                              const std::string &remoteEndpointType,
                                                              const std::string &remoteAddress, int remotePort) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_3RD_PARTY_REMOTE_LISTEN_THEN_DIAL);
    sm.addMember("reqEndpointType", requesterEndpointType);
    sm.addMember("remoteEndpointType", remoteEndpointType);
    sm.addMember("remoteAddress", remoteAddress);
    sm.addMember("port", remotePort);

    sendRequest(requesterAddress, sm);
}

void BackgroundRequester::request3rdPartyListenThenRemoteDial(const std::string &listenAddress,
                                                              const std::string &listenEndpointType,
                                                              const std::string &dialEndpointType,
                                                              const std::string &dialerAddress) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_3RD_PARTY_LOCAL_LISTEN_THEN_REMOTE_DIAL);
    sm.addMember("listenEndpointType", listenEndpointType);
    sm.addMember("dialEndpointType", dialEndpointType);
    sm.addMember("dialerAddress", dialerAddress);

    sendRequest(listenAddress, sm);
}

void BackgroundRequester::requestChangeEndpoint(const std::string &componentAddress, SocketType socketType,
                                                const std::string &endpointType, EndpointSchema *receiverSchema,
                                                EndpointSchema *sendSchema) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CHANGE_ENDPOINT_SCHEMA);
    sm.addMember("endpointType", endpointType);
    if (receiverSchema == nullptr) { sm.setNull("receiveSchema"); }
    else {
        auto schemaObj = receiverSchema->getSchemaObject();
        sm.addMoveObject("receiveSchema", std::move(schemaObj));
    }
    if (sendSchema == nullptr) { sm.setNull("sendSchema"); }
    else {
        auto schemaObj = sendSchema->getSchemaObject();
        sm.addMoveObject("sendSchema", std::move(schemaObj));
    }
    sm.addMember("socketType", socketType);


    sendRequest(componentAddress, sm);
}

int BackgroundRequester::requestCreateRDH(const std::string &componentUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CREATE_RDH);

    auto reply = sendRequest(componentUrl, sm);
    try {
        return reply->getInteger("port");
    } catch (AccessError &e) {
        throw ValidationError("No port number in returned message");
    }
}

// Return empty if error reply
std::vector<std::string> BackgroundRequester::requestLocationsOfRDH(const std::string &componentUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_GET_LOCATIONS_OF_RDH);

    auto reply = sendRequest(componentUrl, sm);
    std::vector<std::string> locations;;
    try {
        locations = reply->getArray<std::string>("locations");
    } catch (AccessError &e) {
        throw ValidationError("Returned message had no appropriate \"locations\"");
    }
    return locations;

}

void BackgroundRequester::requestCloseSocketOfType(const std::string &componentUrl, const std::string &endpointType) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CLOSE_SOCKETS);
    sm.addMember("endpointType", endpointType);

    sendRequest(componentUrl, sm);

}

void
BackgroundRequester::requestUpdateComponentManifest(const std::string &componentUrl, ComponentManifest &newManifest) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CHANGE_MANIFEST);
    auto compRep = newManifest.getSocketMessageCopy();
    sm.addMoveObject("newManifest", std::move(compRep));

    sendRequest(componentUrl, sm);
}

void BackgroundRequester::requestCloseRDH(const std::string &componentUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CLOSE_RDH);

    sendRequest(componentUrl, sm);

}

std::vector<std::unique_ptr<EndpointMessage>> BackgroundRequester::requestEndpointInfo(const std::string &componentUrl) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_REQUEST_ENDPOINT_INFO);
    auto reply = sendRequest(componentUrl, sm);

    return reply->getArray<std::unique_ptr<EndpointMessage>>("endpoints");
}

std::unique_ptr<ComponentManifest> BackgroundRequester::requestComponentManifest(const std::string& componentUrl){
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_GET_COMPONENT_MANIFEST);
    auto reply = sendRequest(componentUrl,sm);

    try {
        std::unique_ptr<ComponentManifest> returnManifest =
                std::make_unique<ComponentManifest>(reply->getMoveObject("manifest").get(),
                                                    component->getSystemSchemas());
        return returnManifest;
    }catch(std::logic_error &e){
        throw ValidationError("Returned Component Manifest was not valid\nReply: " + reply->stringify());
    }
}

void BackgroundRequester::requestCloseSocketOfId(const std::string &componentUrl, const std::string &endpointId) {
    EndpointMessage sm;
    sm.addMember("requestType", BACKGROUND_CLOSE_ENDPOINT_BY_ID);
    sm.addMember("endpointId", endpointId);

    sendRequest(componentUrl, sm);

}
