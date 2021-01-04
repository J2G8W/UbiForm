#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H

#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <map>
#include "rapidjson/schema.h"
#include "rapidjson/error/en.h"

#include "general_functions.h"
#include "EndpointSchema.h"
#include "SystemSchemas/SystemSchemas.h"

/**
 * Used to represent the manifest (that is the description) of a component
 */
class ComponentManifest {
protected:
    rapidjson::Document JSON_document;

    // TODO -  combine into one map at some point
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

    // TODO - don't copy, use move constructors - far more efficient
    /**
     * @brief This is a copy constructor not a move
     *
     * @param sm - SocketMessage pointer to form manifest from, key for network interactions
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    explicit ComponentManifest(SocketMessage* sm,SystemSchemas &es ) : ComponentManifest(sm->stringify().c_str(), es) {
    }


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

    std::string stringify() { return stringifyDocument(JSON_document); };


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @param receiveSchema - whether we want receive or send schema for that type
     * @return SocketMessage pointer which needs memory handling, used for sending our schema on the network
     */
    SocketMessage * getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema);

    ~ComponentManifest();
};


#endif //UBIFORM_COMPONENTMANIFEST_H
