#ifndef UBIFORM_SYSTEMSCHEMAS_H
#define UBIFORM_SYSTEMSCHEMAS_H

#include <memory>
#include <map>
#include "../SchemaRepresentation/EndpointSchema.h"
#include "../SchemaRepresentation/GenericSchema.h"
#include "../Utilities/SystemEnums.h"

class SystemSchemas{
private:
    std::map<SystemSchemaName, std::shared_ptr<GenericSchema> > systemSchemas;
public:
    static int numberOfInstances;
    SystemSchemas(){
        const char* files[12] = {"SystemSchemas/component_schema.json",
                                "SystemSchemas/endpoint_creation_request.json",
                                "SystemSchemas/endpoint_creation_response.json",
                                "SystemSchemas/resource_discovery_addition_request.json",
                                "SystemSchemas/resource_discovery_addition_response.json",
                                "SystemSchemas/resource_discovery_by_id_request.json",
                                "SystemSchemas/resource_discovery_by_id_response.json",
                                "SystemSchemas/resource_discovery_by_schema_request.json",
                                "SystemSchemas/resource_discovery_by_schema_response.json",
                                "SystemSchemas/resource_discovery_component_ids_request.json",
                                "SystemSchemas/resource_discovery_component_ids_response.json",
                                "SystemSchemas/resource_discovery_update_request.json"};

        for (int i =0; i < 12; i++){
            FILE* pFile = fopen(files[i], "r");
            if (pFile == nullptr){
                std::string errorMsg = "Error opening file - " + std::string(files[i]);
                throw AccessError(errorMsg);
            }
            auto es = std::make_shared<GenericSchema>(pFile);
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
