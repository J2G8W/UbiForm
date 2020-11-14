#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"

#include <iostream>
#include <memory>

#include "ComponentManifest.h"

class Component{
private:
    std::unique_ptr<ComponentManifest> manifest;
    nng_socket socket;

public:
    Component():manifest(nullptr){ }
    void specifyManifest(FILE* jsonFP){manifest = std::unique_ptr<ComponentManifest>(new ComponentManifest(jsonFP));}
    void specifyManifest(const char *jsonString){manifest = std::unique_ptr<ComponentManifest>(new ComponentManifest(jsonString));}

    void createPairConnectionOutgoing(const char* url);
    void createPairConnectionIncoming(const char* url);

    void sendManifestOnSocket();
    void receiveManifestOnSocket();

    ~Component();



};


#endif //UBIFORM_COMPONENT_H
