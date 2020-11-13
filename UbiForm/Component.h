#ifndef UBIFORM_COMPONENT_H
#define UBIFORM_COMPONENT_H

#include "rapidjson/document.h"

#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"

#include <iostream>

#include "ComponentManifest.h"

class Component{
private:
    ComponentManifest manifest;
    nng_socket socket;

public:
    void specifyManifest(FILE* jsonFP){manifest  = ComponentManifest(jsonFP);}
    void specifyManifest(const char *jsonString){manifest = ComponentManifest(jsonString);}

    void createPairConnectionOutgoing(const char* url);
    void createPairConnectionIncoming(const char* url);

    void sendManifestOnSocket();
    void receiveManifestOnSocket();



};


#endif //UBIFORM_COMPONENT_H
