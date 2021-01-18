#ifndef UBIFORM_SYSTEMSCHEMAS_H
#define UBIFORM_SYSTEMSCHEMAS_H

#include <memory>
#include <map>
#include "../SchemaRepresentation/EndpointSchema.h"
#include "../SchemaRepresentation/GenericSchema.h"
#include "../Utilities/SystemEnums.h"

class SystemSchemas {
private:
    std::map<SystemSchemaName, std::shared_ptr<GenericSchema> > systemSchemas;
public:
    static int numberOfInstances;

    /**
     * Initiates our SystemSchemas object from an array of pre-programmed files
     * @throws AccessError when we can't find a file
     */
    SystemSchemas() {
        const char *files[16] = {"SystemSchemas/component_schema.json",
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
                                 "SystemSchemas/resource_discovery_update_request.json",
                                 "SystemSchemas/general_resource_discovery_request.json",
                                 "SystemSchemas/general_resource_discovery_response.json",
                                 "SystemSchemas/general_endpoint_request.json",
                                 "SystemSchemas/general_endpoint_response.json"};

        for (int i = 0; i < 16; i++) {
            FILE *pFile = fopen(files[i], "r");
            if (pFile == nullptr) {
                std::string errorMsg = "Error opening file - " + std::string(files[i]);
                auto es = std::make_shared<GenericSchema>();
                systemSchemas.insert(std::make_pair(static_cast<SystemSchemaName>(i), es));
            }else {
                auto es = std::make_shared<GenericSchema>(pFile);
                systemSchemas.insert(std::make_pair(static_cast<SystemSchemaName>(i), es));
                fclose(pFile);
            }
        }

        SystemSchemas::numberOfInstances++;
        //std::cout << "NUM INSTANCES" << numberOfInstances << std::endl;
    }

    /// Used to make sure we are copying a reference rather than a whole object(due to size of thing)
    SystemSchemas(const SystemSchemas &obj) { if (this != &obj) ++numberOfInstances; }

    /**
     * Get the schema object of some Name. There should not be any access errors as our initalisation should cover all the enums
     * @param name - Request schema
     * @return Reference to a schema object
     */
    GenericSchema &getSystemSchema(SystemSchemaName name) {
        return *(systemSchemas.at(name));
    }
};


#endif //UBIFORM_SYSTEMSCHEMAS_H
