#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"

#include <iostream>
#include <memory>

#include "ComponentManifest.h"
#include "SocketMessage.h"

class Component{
private:
    std::unique_ptr<ComponentManifest> componentManifest{nullptr};
    std::unique_ptr<ComponentManifest> socketManifest{nullptr};
    nng_socket socket{};


public:
    Component()= default;
    void specifyManifest(FILE* jsonFP){ componentManifest = std::make_unique<ComponentManifest>(jsonFP);}
    void specifyManifest(const char *jsonString){ componentManifest = std::make_unique<ComponentManifest>(jsonString);}

    void createPairConnectionOutgoing(const char* url);
    void createPairConnectionIncoming(const char* url);

    void sendManifestOnSocket();
    void receiveManifestOnSocket();

    void sendMessage(SocketMessage& s);
    SocketMessage* receiveMessage();

    ~Component();



};


#endif //UBIFORM_COMPONENT_H
