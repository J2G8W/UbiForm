#ifndef UBIFORM_COMPONENTREPRESENTATION_H
#define UBIFORM_COMPONENTREPRESENTATION_H

#include "../ComponentManifest.h"


// NOTE THAT THESE ARE ALSO DEFINED IN "SystemSchemas/resource_discovery_*_request.json"
#define ADDITION "addition"
#define REQUEST_BY_ID "requestId"
#define REQUEST_BY_SCHEMA "requestSchema"
#define REQUEST_COMPONENTS "requestComponents"
#define UPDATE "update"


class ComponentRepresentation : public ComponentManifest{
private:
    std::string url;
    std::vector<std::string> urls;
    int port;
public:
    explicit ComponentRepresentation(const char *JSON_input, SystemSchemas& ss): ComponentManifest(JSON_input,ss){
        fillSelf();
    }
    explicit ComponentRepresentation(SocketMessage * sm, SystemSchemas& ss) : ComponentManifest(sm, ss){
        fillSelf();
    }
    explicit ComponentRepresentation(FILE *jsonFP, SystemSchemas& ss) : ComponentManifest(jsonFP, ss){
        fillSelf();
    }

    void fillSelf();

    std::string getUrl(){return url;}

    std::vector<std::string>& getAllUrls(){return urls;}
    int getPort(){return port;}

    bool isEqual(const std::string& endpointId,bool recv, SocketMessage &sm);

    std::vector<std::string> findEquals(bool recv, SocketMessage &sm);

    SocketMessage * getSchema(const std::string& endpointId, bool recv);


};



#endif //UBIFORM_COMPONENTREPRESENTATION_H
