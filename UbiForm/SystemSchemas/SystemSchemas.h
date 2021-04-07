#ifndef UBIFORM_SYSTEMSCHEMAS_H
#define UBIFORM_SYSTEMSCHEMAS_H

#include <memory>
#include <map>
#include "../../include/UbiForm/SchemaRepresentation/EndpointSchema.h"
#include "../SchemaRepresentation/GenericSchema.h"
#include "../../include/UbiForm/Utilities/SystemEnums.h"

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
        const char *files[16] = {"/component_schema.json",
                                 "/endpoint_creation_request.json",
                                 "/endpoint_creation_response.json",
                                 "/resource_discovery_addition_request.json",
                                 "/resource_discovery_addition_response.json",
                                 "/resource_discovery_by_id_request.json",
                                 "/resource_discovery_by_id_response.json",
                                 "/resource_discovery_by_schema_request.json",
                                 "/resource_discovery_by_schema_response.json",
                                 "/resource_discovery_component_ids_request.json",
                                 "/resource_discovery_component_ids_response.json",
                                 "/resource_discovery_update_request.json",
                                 "/general_resource_discovery_request.json",
                                 "/general_resource_discovery_response.json",
                                 "/general_endpoint_request.json",
                                 "/general_endpoint_response.json"};

        for (int i = 0; i < 16; i++) {
            std::string file = SYSTEM_SCHEMAS_LOC + std::string(files[i]);
            FILE *pFile = fopen(file.c_str(), "r");
            if (pFile == nullptr) {
                std::string errorMsg = "Error opening file - " + std::string(files[i]);
                std::cerr << errorMsg << std::endl;
                auto es = std::make_shared<GenericSchema>();
                systemSchemas.insert(std::make_pair(static_cast<SystemSchemaName>(i), es));
            } else {
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
