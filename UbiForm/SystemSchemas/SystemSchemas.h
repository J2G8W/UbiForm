#ifndef UBIFORM_SYSTEMSCHEMAS_H
#define UBIFORM_SYSTEMSCHEMAS_H

#include <memory>
#include <map>
#include "../endpoints/EndpointSchema.h"
#include "../GenericSchema.h"

enum SystemSchemaName{
    componentManifest,endpointCreationRequest, endpointCreationResponse,
    additionRequest,additionResponse,byIdRequest,byIdResponse,
    bySchemaRequest, bySchemaResponse, componentIdsRequest, componentIdsResponse,
};

class SystemSchemas{
private:
    std::map<SystemSchemaName, GenericSchema *> systemSchemas;
public:
    static int numberOfInstances;
    SystemSchemas(){
        const char* files[11] = {"SystemSchemas/component_schema.json",
                                "SystemSchemas/endpoint_creation_request.json",
                                "SystemSchemas/endpoint_creation_response.json",
                                "SystemSchemas/resource_discovery_addition_request.json",
                                "SystemSchemas/resource_discovery_addition_response.json",
                                "SystemSchemas/resource_discovery_by_id_request.json",
                                "SystemSchemas/resource_discovery_by_id_response.json",
                                "SystemSchemas/resource_discovery_by_schema_request.json",
                                "SystemSchemas/resource_discovery_by_schema_response.json",
                                "SystemSchemas/resource_discovery_component_ids_request.json",
                                "SystemSchemas/resource_discovery_component_ids_response.json"};

        for (int i =0; i < 11; i++){
            FILE* pFile = fopen(files[i], "r");
            if (pFile == nullptr){
                std::cerr << "Error finding requisite file - " << files[i] << std::endl;
                exit(1);
            }
            auto* es = new GenericSchema(pFile);
            systemSchemas.insert(std::make_pair(static_cast<SystemSchemaName>(i), es));
            fclose(pFile);
        }

        SystemSchemas::numberOfInstances ++;
        //std::cout << "NUM INSTANCES" << numberOfInstances << std::endl;
    }

    SystemSchemas(const SystemSchemas& obj) {if(this != &obj) ++numberOfInstances;}

    GenericSchema & getSystemSchema(SystemSchemaName name) {
        return *(systemSchemas.at(name));
    }
};


#endif //UBIFORM_SYSTEMSCHEMAS_H
