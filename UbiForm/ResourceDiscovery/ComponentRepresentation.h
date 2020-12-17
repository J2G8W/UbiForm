#ifndef UBIFORM_COMPONENTREPRESENTATION_H
#define UBIFORM_COMPONENTREPRESENTATION_H

#include "../ComponentManifest.h"

#define ADDITION "addition"
#define REQUEST_BY_ID "requestId"
#define REQUEST_BY_SCHEMA "requestSchema"
#define REQUEST_COMPONENTS "requestComponents"


class ComponentRepresentation : public ComponentManifest{
private:
    std::string url;
public:
    explicit ComponentRepresentation(const char *JSON_input): ComponentManifest(JSON_input){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }
    explicit ComponentRepresentation(SocketMessage * sm) : ComponentManifest(sm){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }
    explicit ComponentRepresentation(FILE *jsonFP) : ComponentManifest(jsonFP){
        if(JSON_document.HasMember("url") && JSON_document["url"].IsString()){
            url = JSON_document["url"].GetString();
        }else{
            throw std::logic_error("NO URL IN JSON");
        }
    }

    std::string getUrl(){return url;}

    bool isEqual(std::string endpointId,bool recv, SocketMessage &sm);

    std::vector<std::string> findEquals(bool recv, SocketMessage &sm);

    SocketMessage * getSchema(std::string endpointId, bool recv);


};



#endif //UBIFORM_COMPONENTREPRESENTATION_H
