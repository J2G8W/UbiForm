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

    SystemSchemas &systemSchemas;

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
    ComponentManifest(const char *jsonString, SystemSchemas &es);

    /**
     * @param jsonFP - FILE pointer to form manifest from, used instead of file streams as get
     * better performance from RapidJson
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    ComponentManifest(FILE *jsonFP, SystemSchemas &es);


    /**
     * @brief This is a copy constructor not a move
     *
     * @param sm - SocketMessage pointer to form manifest from, key for network interactions
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    ComponentManifest(SocketMessage *sm, SystemSchemas &ss);

    /// Create an empty Manifest which can be filled with functions
    explicit ComponentManifest(SystemSchemas &ss) : ComponentManifest(R"({"name":"","schemas":{}})", ss) {};

    /// Copies the input rather than moving it
    void setManifest(FILE *jsonFP);

    void setManifest(const char *jsonString);

    void setManifest(SocketMessage *sm);

    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return pointer to EndpointSchema for that typeOfEndpoint
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::shared_ptr<EndpointSchema> getReceiverSchema(const std::string &typeOfEndpoint);

    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return pointer to EndpointSchema for that typeOfEndpoint
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::shared_ptr<EndpointSchema> getSenderSchema(const std::string &typeOfEndpoint);

    /**
     * @param name - Input name to be set
     */
    void setName(const std::string &name);

    /**
     * @return The name attached to the componentManifest
     */
    std::string getName();

    void setProperty(const std::string &propertyName, const std::string &value);

    std::string getProperty(const std::string &propertyName);

    void removeProperty(const std::string &propertyName);

    bool hasProperty(const std::string &propertyName);

    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return string of what type the socket is (e.g. pair)
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::string getSocketType(const std::string &endpointType);

    /**
     * @return string representation of the component manifest
     */
    std::string stringify() { return stringifyValue(JSON_document); };


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @param receiveSchema - whether we want receive or send schema for that type
     * @return SocketMessage pointer which needs memory handling, used for sending our schema on the network
     */
    SocketMessage *getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema);


    /**
     * Note that this copies from the schemas given to it, it doesn't use the same schemas
     * @param socketType - specify the socketType of the new endpoint
     * @param typeOfEndpoint - specify the name of the endpoint (will replace previous endpoints of same name
     * @param receiveSchema - either a pointer to the schema for the endpoint or nullptr if relevant socketType doesn't need receiveSchema
     * @param sendSchema - either a pointer to the schema for the endpoint of nullptr if relevant socketType doens't need sendSchema
     * @throws std::logic_error - when there aren't enough schemas given for the socketType
     */
    void addEndpoint(SocketType socketType, const std::string &typeOfEndpoint,
                     std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema);


    /**
     * We get a copy of the manifest but as a SocketMessage, such that it can be sent on the wire
     * @return - std::unique_ptr used such that we get automatic memory handling and move's work better
     */
    std::unique_ptr<SocketMessage> getSocketMessageCopy() {
        // Gets around private constructor
        return std::unique_ptr<SocketMessage>(new SocketMessage(this->JSON_document, true));
    }

    ~ComponentManifest();
};


#endif //UBIFORM_COMPONENTMANIFEST_H
