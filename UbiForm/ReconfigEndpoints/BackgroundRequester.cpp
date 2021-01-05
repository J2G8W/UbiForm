// Created by julian on 05/01/2021.
//

#include <string>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include "BackgroundRequester.h"
#include "BackgroundListener.h"
#include "../Component.h"

std::string BackgroundRequester::requestConnection(const std::string &address, const std::string& requestText) {
    int rv;

    nng_socket tempSocket;
    if ((rv = nng_req0_open(&tempSocket)) != 0) {
        throw NngError(rv, "Opening temporary socket for connection request");
    }

    if ((rv = nng_dial(tempSocket, address.c_str(), nullptr, 0)) != 0) {
        throw NngError(rv, "Dialing " + address + " for connection request");
    }

    if ((rv = nng_send(tempSocket, (void *) requestText.c_str(), requestText.size() + 1, 0)) != 0) {
        throw NngError(rv, "Sending to " + address + " for connection request");
    }

    char *buf = nullptr;
    size_t sz;
    if ((rv = nng_recv(tempSocket, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
        throw NngError(rv, "Receiving response from connection request");
    }
    try{
        SocketMessage response(buf);
        nng_free(buf,sz);

        if (response.isNull("url")){
            throw std::logic_error("No valid endpoint of: " + requestText);
        }else{
            try {
                // Basically validation of response (CBA for using schemas for one field)
                return response.getString("url");
            }catch (AccessError &e){
                throw std::logic_error("No endpoint returned");
            }
        }
    } catch (std::logic_error &e) {
        std::cerr << "Error in handling response" << std::endl;
        throw;
    }
}

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

    systemSchemas.getSystemSchema(SystemSchemaName::endpointCreationRequest).validate(sm);

    std::string url;
    try{
        url = requestConnection(connectionComponentAddress,sm.stringify());
        component->createEndpointAndDial(requestSocketType, localEndpointType, url);
    }catch (std::logic_error &e){
        std::cerr << e.what() << std::endl;
    }
}