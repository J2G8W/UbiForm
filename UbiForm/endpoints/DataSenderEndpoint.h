#ifndef UBIFORM_DATASENDERENDPOINT_H
#define UBIFORM_DATASENDERENDPOINT_H

#include <memory>

#include <nng/nng.h>
#include "../SocketMessage.h"

#include "EndpointSchema.h"

class DataSenderEndpoint {
protected:
    // Socket is initialised in extending class
    nng_socket * senderSocket = nullptr;
    std::shared_ptr<EndpointSchema>senderSchema;
    std::string listenUrl;
public:
    explicit DataSenderEndpoint( std::shared_ptr<EndpointSchema>& es){
        senderSchema = es;
    };

    // This is implemented by extending classes as we want to specify socket type and do other useful things
    virtual void listenForConnection(const char *url) = 0;


    void sendMessage(SocketMessage &s);
    std::string getListenUrl(){return listenUrl;}
};


#endif //UBIFORM_DATASENDERENDPOINT_H
