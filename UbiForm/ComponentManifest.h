#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H

#include <map>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include "rapidjson/schema.h"
#include "rapidjson/error/en.h"

#include "Utilities/UtilityFunctions.h"
#include "SchemaRepresentation/EndpointSchema.h"
#include "SystemSchemas/SystemSchemas.h"
#include "Utilities/SystemEnums.h"

/**
 * Used to represent the manifest (that is the description) of a component
 */
class ComponentManifest {
protected:
    rapidjson::Document JSON_document;

    // Note that these maps will "auto delete" as we are using shared pointers so we don't need to worry about their memory
    std::map<std::string, std::shared_ptr<EndpointSchema> > receiverSchemas;
    std::map<std::string, std::shared_ptr<EndpointSchema> > senderSchemas;

    SystemSchemas & systemSchemas;

    void checkParse();
    void fillSchemaMaps();

public:


    /**
     * @param jsonString - text to form manifest from
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    explicit ComponentManifest(const char *jsonString, SystemSchemas & es);

    /**
     * @param jsonFP - FILE pointer to form manifest from, used instead of file streams as get
     * better performance from RapidJson
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    explicit ComponentManifest(FILE *jsonFP, SystemSchemas &es);


    /**
     * @brief This is a copy constructor not a move
     *
     * @param sm - SocketMessage pointer to form manifest from, key for network interactions
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    explicit ComponentManifest(SocketMessage* sm,SystemSchemas &ss );


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return pointer to EndpointSchema for that typeOfEndpoint
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::shared_ptr<EndpointSchema> getReceiverSchema(const std::string& typeOfEndpoint);
    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return pointer to EndpointSchema for that typeOfEndpoint
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::shared_ptr<EndpointSchema> getSenderSchema(const std::string& typeOfEndpoint);

    std::string getName();


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return string of what type the socket is (e.g. pair)
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::string getSocketType(const std::string& endpointType);

    /**
     * @return string representation of the component manifest
     */
    std::string stringify() { return stringifyValue(JSON_document); };


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @param receiveSchema - whether we want receive or send schema for that type
     * @return SocketMessage pointer which needs memory handling, used for sending our schema on the network
     */
    SocketMessage * getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema);


    /**
     * @param socketType - specify the socketType of the new endpoint
     * @param typeOfEndpoint - specify the name of the endpoint (will replace previous endpoints of same name
     * @param receiveSchema - either a pointer to the schema for the endpoint or nullptr if relevant socketType doesn't need receiveSchema
     * @param sendSchema - either a pointer to the schema for the endpoint of nullptr if relevant socketType doens't need sendSchema
     * @throws std::logic_error - when there aren't enough schemas given for the socketType
     */
    void addSchema(SocketType socketType, const std::string& typeOfEndpoint,
                   std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema);


    SocketMessage* getComponentRepresentation(){
        return new SocketMessage(this->JSON_document);
    }

    ~ComponentManifest();
};


#endif //UBIFORM_COMPONENTMANIFEST_H
