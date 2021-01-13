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
    auto sm = component->getComponentManifest().getSocketMessageCopy();
    sm->addMember("urls",component->getAllAddresses());
    sm->addMember("port",component->getBackgroundPort());

    request->addMoveObject("manifest", std::move(sm));
    return request;
}

void ResourceDiscoveryConnEndpoint::registerWithHub(const std::string& url) {
    auto request = std::unique_ptr<SocketMessage>(generateRegisterRequest());

    systemSchemas.getSystemSchema(SystemSchemaName::additionRequest).validate(*request);
    std::unique_ptr<SocketMessage> reply;
    try {
        reply = sendRequest(url, request.get());
        systemSchemas.getSystemSchema(SystemSchemaName::additionResponse).validate(*reply);
        std::cout << "Registered successfully with: " << url << " id is " << reply->getString("newID") << std::endl;
    }catch(std::logic_error &e){
        std::cerr << "Error registering with " << url << "\n\t" << e.what() << std::endl;
        throw;
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
        return std::vector<std::string>();
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

SocketMessage *ResourceDiscoveryConnEndpoint::generateFindBySchemaRequest(const std::string& endpointType,
                                                                          std::map<std::string, std::string> &otherValues) {
    auto * request = new SocketMessage;
    request->addMember("request",REQUEST_BY_SCHEMA);

    // We want the schema we get back to be a data SENDER
    request->addMember("dataReceiverEndpoint",false);

    // We want our schema to be receiving data
    auto schema = std::unique_ptr<SocketMessage>(component->getComponentManifest().getSchemaObject(endpointType, true));

    request->addMoveObject("schema", std::move(schema));

    auto specialProperties = std::make_unique<SocketMessage>();
    for(auto& keyValuePair : otherValues){
        specialProperties->addMember(keyValuePair.first,keyValuePair.second);
    }
    request->addMoveObject("specialProperties", std::move(specialProperties));

    return request;
}

std::vector<std::unique_ptr<SocketMessage>>
ResourceDiscoveryConnEndpoint::getComponentsBySchema(const std::string &endpointType,
                                                     std::map<std::string, std::string> &otherValues) {
    std::vector<std::unique_ptr<SocketMessage>> returnEndpoints;

    SocketMessage* request = generateFindBySchemaRequest(endpointType, otherValues);

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

        //returnEndpoints.insert(
        //        returnEndpoints.end(),
        //        std::make_move_iterator(replyEndpoints.begin()),
        //        std::make_move_iterator(replyEndpoints.end())
        //);

        for (auto s : replyEndpoints){
            returnEndpoints.push_back(std::unique_ptr<SocketMessage>(s));
        }
    }
    delete request;
    return returnEndpoints;
}


void ResourceDiscoveryConnEndpoint::createEndpointBySchema(const std::string& endpointType){
    std::map<std::string,std::string> empty;
    std::vector<std::unique_ptr<SocketMessage>> validLocations = getComponentsBySchema(endpointType, empty);

    for (const auto & location: validLocations) {
        bool connection = false;
        for (const auto & url: location->getArray<std::string>("urls")) {
            try {
                component->getBackgroundRequester().requestAndCreateConnection(url, location->getInteger("port"),
                                                                               endpointType,
                                                                               location->getString("endpointType"));
                connection = true;
                break;
            }catch(std::logic_error &e){
                // PASS
            }
        }
        if (!connection) {
            std::cerr << "Error connecting to " << location->stringify() << std::endl;
        }
    }
}

void ResourceDiscoveryConnEndpoint::updateManifestWithHubs() {
    auto newManifest = component->getComponentManifest().getSocketMessageCopy();
    newManifest->addMember("urls",component->getAllAddresses());
    newManifest->addMember("port",component->getBackgroundPort());

    auto request = std::make_unique<SocketMessage>();
    request->addMember("request",UPDATE);
    request->addMoveObject("newManifest", std::move(newManifest));

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

void ResourceDiscoveryConnEndpoint::searchForResourceDiscoveryHubs() {
    auto addresses = component->getAllAddresses();
    bool found = false;
    if (component->getComponentConnectionType() == ConnectionType::IPC){
        std::cerr << "Can't search for RDH's with IPC" << std::endl;
        return;
    }
    for(const auto& address: addresses){
        if(found){break;}

        std::string subnet = address.substr(0,address.rfind('.'));
        for(int i = 0;i <= 255; i++){
            if(found){break;}
            std::string dialAddress = subnet +"." +  std::to_string(i) + ":" + std::to_string(DEFAULT_BACKGROUND_LISTEN_PORT);
            try{
                std::vector<std::string> RDHs =
                        component->getBackgroundRequester().requestLocationsOfRDH(dialAddress);
                if(RDHs.empty()){continue;}
                for(const std::string& url : RDHs){
                    try {
                        registerWithHub(url);
                        found = true;
                        std::cout << "Found Resource Discovery Hub: " << url << std::endl;
                        break;
                    }catch(std::logic_error &e){
                        //IGNORED
                    }
                }
            }catch(std::logic_error &e){
                // IGNORED
            }
        }
    }
}
