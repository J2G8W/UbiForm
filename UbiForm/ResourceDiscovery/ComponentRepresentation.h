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
public:
    explicit ComponentRepresentation(const char *JSON_input, SystemSchemas& ss): ComponentManifest(JSON_input,ss){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }
    explicit ComponentRepresentation(SocketMessage * sm, SystemSchemas& ss) : ComponentManifest(sm, ss){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }
    explicit ComponentRepresentation(FILE *jsonFP, SystemSchemas& ss) : ComponentManifest(jsonFP, ss){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }

    std::string getUrl(){return url;}

    bool isEqual(const std::string& endpointId,bool recv, SocketMessage &sm);

    std::vector<std::string> findEquals(bool recv, SocketMessage &sm);

    SocketMessage * getSchema(const std::string& endpointId, bool recv);


};



#endif //UBIFORM_COMPONENTREPRESENTATION_H
