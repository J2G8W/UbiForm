#include "ResourceDiscoveryConnEndpoint.h"
#include "../Component.h"
#include <nng/protocol/reqrep0/req.h>
std::unique_ptr<SocketMessage> ResourceDiscoveryConnEndpoint::sendRequest(const std::string& url, SocketMessage *request) {

    int rv;
    nng_socket requestSocket;
    if ((rv = nng_req0_open(&requestSocket)) != 0) {
        throw NngError(rv, "Open RD connection request socket");
    }

    if ((rv = nng_dial(requestSocket, url.c_str(), nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing RD hub at " + url);
    }

    std::string reqText = request->stringify();
    if ((rv = nng_send(requestSocket,(void*)reqText.c_str(),reqText.size() +1,0)) !=0 ){
        throw NngError(rv, "Sending message to RD hub at " + url);
    }
    char *buf;
    size_t sz;
    if ((rv = nng_recv(requestSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0){
        throw NngError(rv, "Error receiving request from RD hub at " + url);
    }
    try{
        auto replyMsg = std::make_unique<SocketMessage>(buf);
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
    auto sm = component->getComponentManifest().getComponentRepresentation();
    sm->addMember("url",component->getBackgroundListenAddress());

    request->moveMember("manifest",std::move(sm));
    return request;
}

void ResourceDiscoveryConnEndpoint::registerWithHub(const std::string& url) {
    auto request = std::unique_ptr<SocketMessage>(generateRegisterRequest());

    systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*request);
    std::unique_ptr<SocketMessage> reply;
    try {
        reply = sendRequest(url, request.get());
        systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
    }catch(std::logic_error &e){
        std::cerr << "Error registering with " << url << "\n\t" << e.what() << std::endl;
        return;
    }

    resourceDiscoveryHubs.insert(std::pair<std::string,std::string>(url, reply->getString("newID")));
}

std::vector<std::string> ResourceDiscoveryConnEndpoint::getComponentIdsFromHub(const std::string& url) {
    SocketMessage request;
    request.addMember("request",REQUEST_COMPONENTS);

    systemSchemas.getSystemSchema(SystemSchemaName::componentIdsRequest).validate(request);
    std::unique_ptr<SocketMessage> reply;

    try {
        reply = sendRequest(url, &request);
        systemSchemas.getSystemSchema(SystemSchemaName::componentIdsResponse).validate(*reply);
    }catch(std::logic_error &e){
        std::cerr << "Error getting component ids from " << url << "\n\t" << e.what() << std::endl;
    }

    // This is a copy constructor
    return reply->getArray<std::string>("components");
}

std::unique_ptr<ComponentRepresentation> ResourceDiscoveryConnEndpoint::getComponentById(const std::string& url, const std::string& id) {
    SocketMessage request;
    request.addMember("request", REQUEST_BY_ID);
    request.addMember("id",id);

    systemSchemas.getSystemSchema(SystemSchemaName::byIdRequest).validate(request);


    std::unique_ptr<SocketMessage> reply;
    try {
        reply = sendRequest(url, &request);
        systemSchemas.getSystemSchema(SystemSchemaName::byIdResponse).validate(*reply);
    }catch(std::logic_error &e){
        throw;
    }

    if (reply->isNull("component")){
        throw std::logic_error("RDH did not have a component of that ID");
    }
    // Here we check if the ComponentRepresentation returned is a valid ComponentRepresentation
    try{
        // We copy compRep when making the ComponentRepresentation object anyway
        auto compRep = reply->getMoveObject("component");
        auto componentRepresentation = std::make_unique<ComponentRepresentation>(compRep.get(), systemSchemas);
        return componentRepresentation;
    }catch(std::logic_error &e){
        std::cerr << "Malformed reply from RDH" << std::endl;
        throw ValidationError(e.what());
    }
}

SocketMessage *ResourceDiscoveryConnEndpoint::generateFindBySchemaRequest(const std::string& endpointType) {
    auto * request = new SocketMessage;
    request->addMember("request",REQUEST_BY_SCHEMA);

    // We want the schema we get back to be a data SENDER
    request->addMember("dataReceiverEndpoint",false);

    // We want our schema to be receiving data
    auto schema = std::unique_ptr<SocketMessage>(component->getComponentManifest().getSchemaObject(endpointType, true));

    request->moveMember("schema", std::move(schema));
    return request;
}

std::vector<SocketMessage *> ResourceDiscoveryConnEndpoint::getComponentsBySchema(const std::string& endpointType) {
    std::vector<SocketMessage *> returnEndpoints;

    SocketMessage* request = generateFindBySchemaRequest(endpointType);

    systemSchemas.getSystemSchema(SystemSchemaName::bySchemaRequest).validate(*request);

    for (const auto& rdh : resourceDiscoveryHubs){

        std::unique_ptr<SocketMessage> reply;
        try {
            reply = sendRequest(rdh.first, request);
            systemSchemas.getSystemSchema(SystemSchemaName::bySchemaResponse).validate(*reply);
        }catch(std::logic_error &e){
            std::cerr << "Error getting component from " << rdh.first << "\n\t" << e.what() << std::endl;
            continue;
        }

        std::vector<SocketMessage *> replyEndpoints = reply->getArray<SocketMessage *>("endpoints");

        returnEndpoints.insert(
                returnEndpoints.end(),
                std::make_move_iterator(replyEndpoints.begin()),
                std::make_move_iterator(replyEndpoints.end())
        );
    }
    delete request;
    return returnEndpoints;
}


void ResourceDiscoveryConnEndpoint::createEndpointBySchema(const std::string& endpointType){
    std::vector<SocketMessage *> validLocations = getComponentsBySchema(endpointType);

    for (const auto & location: validLocations) {
        try {
            component->getBackgroundRequester().requestAndCreateConnection(location->getString("url"), endpointType,
                                                                           location->getString("endpointType"));
        }catch(std::logic_error &e){
            std::cerr << "Error connecting to " << location << "\n\t" <<e.what() <<std::endl;
        }
    }
}

void ResourceDiscoveryConnEndpoint::updateManifestWithHubs() {
    auto newManifest = component->getComponentManifest().getComponentRepresentation();
    newManifest->addMember("url",component->getBackgroundListenAddress());
    auto request = std::make_unique<SocketMessage>();
    request->addMember("request",UPDATE);
    request->moveMember("newManifest", std::move(newManifest));
    for(auto& locationIdPair : resourceDiscoveryHubs){
        request->addMember("id",locationIdPair.second);
        systemSchemas.getSystemSchema(SystemSchemaName::updateRequest).validate(*request);
        try {
            auto reply = sendRequest(locationIdPair.first, request.get());
            systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
        }catch(std::logic_error &e){
            std::cerr << "Problem connecting to " << locationIdPair.first << "\n\t" << e.what() << std::endl;
        }

    }
}
