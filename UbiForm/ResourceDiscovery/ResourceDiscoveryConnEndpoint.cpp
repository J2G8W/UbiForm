#include "ResourceDiscoveryConnEndpoint.h"
#include "ComponentRepresentation.h"

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
SocketMessage* ResourceDiscoveryConnEndpoint::sendRequest(std::string url, SocketMessage *request) {

    int rv;
    nng_socket requestSocket;
    if ((rv = nng_req0_open(&requestSocket)) != 0) {
        throw NNG_error(rv, "Open RD connection request socket");
    }

    if ((rv = nng_dial(requestSocket, url.c_str(), nullptr, 0)) != 0) {
        throw NNG_error(rv, "Dialing RD hub at " + url);
    }

    std::string reqText = request->stringify();
    if ((rv = nng_send(requestSocket,(void*)reqText.c_str(),reqText.size() +1,0)) !=0 ){
        throw NNG_error(rv, "Sending message to RD hub at " + url);
    }
    char *buf;
    size_t sz;
    if ((rv = nng_recv(requestSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0){
        throw NNG_error(rv, "Error receiving request from RD hub at " + url);
    }
    try{
        auto * replyMsg = new SocketMessage(buf);
        nng_free(buf,sz);
        return replyMsg;
    }catch(ValidationError &e){
        std::cerr << "Error processing reply from RD hub" << std::endl;
        nng_free(buf,sz);
        throw;
    }


}

SocketMessage *ResourceDiscoveryConnEndpoint::generateRegisterRequest() {
    auto * request = new SocketMessage;
    request->addMember("request",ADDITION);
    // TODO - turn into something better
    SocketMessage sm(component->getComponentManifest()->stringify().c_str());
    sm.addMember("url",component->getBackgroundListenAddress());

    request->addMember("manifest",sm);
    return request;
}

void ResourceDiscoveryConnEndpoint::registerWithHub(std::string url) {
    SocketMessage * request = generateRegisterRequest();

    systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*request);

    SocketMessage * reply = sendRequest(url, request);

    systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
    std::cout << reply->getString("newID") << std::endl;
    delete request;
    delete reply;
}

std::vector<std::string> ResourceDiscoveryConnEndpoint::getComponentIdsFromHub(std::string url) {
    SocketMessage request;
    request.addMember("request",REQUEST_COMPONENTS);

    systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(request);

    SocketMessage * reply = sendRequest(url, &request);

    systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*reply);

    return reply->getArray<std::string>("components");
}

ComponentRepresentation *ResourceDiscoveryConnEndpoint::getComponentById(std::string url, std::string id) {
    SocketMessage request;
    request.addMember("request", REQUEST_BY_ID);
    request.addMember("id",id);

    systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(request);

    SocketMessage* reply = sendRequest(url, &request);

    systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*reply);

    if (reply->isNull("component")){
        delete reply;
        throw std::logic_error("RDH did not have a component of that ID");
    }
    try{
        SocketMessage* compRep = reply->getObject("component");
        auto * componentRepresentation = new ComponentRepresentation(compRep, systemSchemas);
        delete compRep;
        delete reply;
        return componentRepresentation;
    }catch(std::logic_error &e){
        std::cerr << "Malformed reply from RDH" << std::endl;
        throw ValidationError(e.what());
    }
}

SocketMessage *ResourceDiscoveryConnEndpoint::generateFindBySchemaRequest(std::string endpointType) {
    auto * request = new SocketMessage;
    request->addMember("request",REQUEST_BY_SCHEMA);

    // We want the schema we get back to be a data SENDER
    request->addMember("dataReceiverEndpoint",false);

    // We want our schema to be receiving data
    SocketMessage* schema = component->getComponentManifest()->getSchemaObject(endpointType, true);

    request->addMember("schema", *schema);
    delete schema;

    return request;
}

std::vector<SocketMessage *> ResourceDiscoveryConnEndpoint::getComponentsBySchema(std::string endpointType) {
    std::vector<SocketMessage *> returnEndpoints;

    SocketMessage* request = generateFindBySchemaRequest(endpointType);

    systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*request);

    for (const auto& rdh : RDHUrls){
        SocketMessage* reply = sendRequest(rdh,request);

        systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*reply);

        std::vector<SocketMessage *> replyEndpoints = reply->getArray<SocketMessage *>("endpoints");
        delete reply;

        returnEndpoints.insert(
                returnEndpoints.end(),
                std::make_move_iterator(replyEndpoints.begin()),
                std::make_move_iterator(replyEndpoints.end())
        );
    }
    delete request;
    return returnEndpoints;
}


void ResourceDiscoveryConnEndpoint::createEndpointBySchema(std::string endpointType){
    std::vector<SocketMessage *> validLocations = getComponentsBySchema(endpointType);

    for (const auto & location: validLocations) {
        component->requestAndCreateConnection(endpointType,location->getString("url"), location->getString("endpointType"));
    }
}