#include "ResourceDiscoveryConnEndpoint.h"
#include "ComponentRepresentation.h"

#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
SocketMessage* ResourceDiscoveryConnEndpoint::sendRequest(std::string url, SocketMessage *request) {

    int rv;
    nng_socket requestSocket;
    if ((rv = nng_req0_open(&requestSocket)) != 0) {
        fatal("Failure opening background socket", rv);
    }

    if ((rv = nng_dial(requestSocket, url.c_str(), nullptr, 0)) != 0) {
        fatal("nng_listen", rv);
    }

    std::string reqText = request->stringify();
    if ((rv = nng_send(requestSocket,(void*)reqText.c_str(),reqText.size() +1,0)) !=0 ){
        fatal("nng_send", rv);
    }
    char *buf;
    size_t sz;
    if ((rv = nng_recv(requestSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0){
        fatal("nng_recv",rv);
    }
    try{
        auto * replyMsg = new SocketMessage(buf);
        nng_free(buf,sz);
        return replyMsg;
    }catch(std::logic_error &e){
        std::cerr << "ERROR ON PROCESSING MESSAGE" <<std::endl;
        std::cerr << e.what() << std::endl;
        return nullptr;
    }


}

SocketMessage *ResourceDiscoveryConnEndpoint::generateRegisterRequest() {
    auto * request = new SocketMessage;
    request->addMember("request",ADDITION);
    // TODO - turn into something better
    SocketMessage sm(component.getComponentManifest()->stringify().c_str());
    sm.addMember("url",component.getBackgroundListenAddress());

    request->addMember("manifest",sm);
    return request;
}

void ResourceDiscoveryConnEndpoint::registerWithHub(std::string url) {
    SocketMessage * request = generateRegisterRequest();
    SocketMessage * reply = sendRequest(url, request);

    std::cout << reply->getInteger("id") << std::endl;
    delete request;
    delete reply;
}

std::vector<std::string> ResourceDiscoveryConnEndpoint::getComponentIdsFromHub(std::string url) {
    SocketMessage request;
    request.addMember("request",REQUEST_COMPONENTS);
    SocketMessage * reply = sendRequest(url, &request);

    return reply->getArray<std::string>("components");
}

ComponentRepresentation *ResourceDiscoveryConnEndpoint::getComponentById(std::string url, std::string id) {
    SocketMessage request;
    request.addMember("request", REQUEST_BY_ID);
    request.addMember("id",id);
    SocketMessage* reply = sendRequest(url, &request);
    if (reply->isNull("component")){
        delete reply;
        throw std::logic_error("RDH did not have a component of that ID");
    }
    try{
        SocketMessage* compRep = reply->getObject("component");
        auto * componentRepresentation = new ComponentRepresentation(compRep);
        delete compRep;
        delete reply;
        return componentRepresentation;
    }catch(std::logic_error &e){
        std::cerr << "Malformed input from RDH" << std::endl;
        throw;
    }
}

SocketMessage *ResourceDiscoveryConnEndpoint::generateFindBySchemaRequest(std::string endpointType) {
    SocketMessage * request = new SocketMessage;
    request->addMember("request",REQUEST_BY_SCHEMA);
    // TODO - CHECK THIS BOOLEAN
    request->addMember("receiveData",false);

    // TODO - figure out how to get schema out
    component.getComponentManifest()->getReceiverSchema(endpointType);
    return nullptr;
}

std::vector<SocketMessage *> ResourceDiscoveryConnEndpoint::getComponentsBySchema(std::string endpointType) {
    return std::vector<SocketMessage *>();
}
