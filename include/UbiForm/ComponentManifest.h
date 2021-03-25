#ifndef UBIFORM_COMPONENTMANIFEST_H
#define UBIFORM_COMPONENTMANIFEST_H

#include <map>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include "rapidjson/schema.h"
#include "rapidjson/error/en.h"

#include "../../UbiForm/Utilities/UtilityFunctions.h"
#include "SchemaRepresentation/EndpointSchema.h"
#include "../../UbiForm/SystemSchemas/SystemSchemas.h"
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

    endpointAdditionCallBack additionCallBack = nullptr;
    void * additionUserData = nullptr;

    manifestChangeCallBack changeCallBack = nullptr;
    void* changeUserData = nullptr;

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
     * @param sm - EndpointMessage pointer to form manifest from, key for network interactions
     * @param es - reference to some SystemSchemas object for validation of manifest
     *
     * @throws ParsingError - when input is malformed
     * @throws ValidationError - when input does not conform to SystemsSchemas
     */
    ComponentManifest(EndpointMessage *sm, SystemSchemas &ss);

    /// Create an empty Manifest which can be filled with functions
    explicit ComponentManifest(SystemSchemas &ss) : ComponentManifest(R"({"name":"","schemas":{}})", ss) {};

    ///@brief Copy constructors for set our manifest
    ///@{
    void setManifest(FILE *jsonFP);

    void setManifest(const char *jsonString);

    void setManifest(EndpointMessage *sm);
    ///@}

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

    /**
     * @brief Set an arbitrary property (as a string) for the manifest
     * @param propertyName - The name of property you want
     * @param value - The value
     * @throws AccessError - When trying to change a reserved name ("urls","port","schemas")
     */
    void setProperty(const std::string &propertyName, const std::string &value);

    /**
     * @brief Get the value of some property in the manifest
     * @param propertyName - The name of the property you want
     * @return A string of the property
     * @throws AccessError - when the Manifest doesn't have the property
     */
    std::string getProperty(const std::string &propertyName);

    /**
     * @brief Remove the property from the manifest
     * @param propertyName - The name of the property
     * @throws AccessError - When trying to remove a reserved name ("urls","port","schemas")
     */
    void removeProperty(const std::string &propertyName);

    /**
     * @brief Check if the manifest has a member (is always a string member)
     * @param propertyName - The name of the property
     * @return Whether the manifest has the member
     */
    bool hasProperty(const std::string &propertyName);

    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @return string of connection paradigm of endpoint (e.g. pair)
     * @throws std::out_of_range - when typeOfEndpoint does not exist
     */
    std::string getConnectionParadigm(const std::string &endpointType);

    /**
     * @return string representation of the component manifest
     */
    std::string stringify() { return stringifyValue(JSON_document); };

    std::string prettyStringify();


    /**
     * @param typeOfEndpoint - specify typeOfEndpoint as described in Manifest
     * @param receiveSchema - whether we want receive or send schema for that type
     * @return EndpointMessage pointer which needs memory handling, used for sending our schema on the network
     */
    std::unique_ptr<EndpointMessage> getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema);


    /**
     * Note that this copies from the schemas given to it, it doesn't use the same schemas
     * @param connectionParadigm - specify the connectionParadigm of the new endpoint
     * @param typeOfEndpoint - specify the name of the endpoint (will replace previous endpoints of same name
     * @param receiveSchema - either a pointer to the schema for the endpoint or nullptr if relevant connectionParadigm doesn't need receiveSchema
     * @param sendSchema - either a pointer to the schema for the endpoint of nullptr if relevant connectionParadigm doens't need sendSchema
     * @throws std::logic_error - when there aren't enough schemas given for the connectionParadigm
     */
    void addEndpoint(ConnectionParadigm connectionParadigm, const std::string &typeOfEndpoint,
                     std::shared_ptr<EndpointSchema> receiveSchema, std::shared_ptr<EndpointSchema> sendSchema);


    /**
     * We get a copy of the manifest but as a EndpointMessage, such that it can be sent on the wire
     * @return - std::unique_ptr used such that we get automatic memory handling and move's work better
     */
    std::unique_ptr<EndpointMessage> getEndpointMessageCopy() {
        // Gets around private constructor
        return std::unique_ptr<EndpointMessage>(new EndpointMessage(this->JSON_document, true));
    }

    /**
     * @brief Gets the name of all the possible endpoints that can be made from the manifest
     * @return A vector of the names
     */
    std::vector<std::string> getAllEndpointTypes();

    /**
     * The listen port is for special Endpoints (Publisher and Reply) where we have a single entity for any given endpointType.
     * Adding the listenPort will specify in the manifest the port to find the given endpoint on
     * @param endpointType
     * @param port
     * @throws AccessError - when the endpointType isn't valid or points to something which isn't Publisher or Reply
     */
    void addListenPort(const std::string &endpointType, int port);

    /**
     * @brief Getter for the listen port of an endpoint
     * @param endpointType
     * @return The port for that endpoint
     * @throws AccessError - when there isn't a port or no endpointType
     */
    int getListenPort(const std::string &endpointType);

    /**
     * @brief Remove the listenPort for some endpoint
     * @param endpointType
     * @throws AccessError
     */
    void removeListenPort(const std::string &endpointType);

    /**
     * @brief Check if endpointType has a listenPort
     * @param endpointType
     * @return
     * @throws AccessError
     */
    bool hasListenPort(const std::string &endpointType);

    /**
     * Simply deletes the underlying JSON object
     */
    ~ComponentManifest();


    /**
     * Returns true if the manifest has an endpoint of the given type
     * @param endpointType - the endpoint requested
     * @return
     */
    bool hasEndpoint(const std::string &endpointType);

    /**
     * Register a callback on the ComponentManifest for when we add or change an endpoint on the manifest.
     * This is designed such that when an endpoint is changed remotely we can respond
     * @param callBack The function to be called on the endpoint addition, the std::string is the "typeOfEndpoint" added
     * and the void* is the data which was provided by the user (note we don't do copying, so pointer must remaind valid)
     * @param providedData The data we want to pass to our callback
     */
    void registerEndpointAdditionCallback(endpointAdditionCallBack callBack, void* providedData);


    void registerManifestChangeCallback(manifestChangeCallBack callBack, void* userData);
};


#endif //UBIFORM_COMPONENTMANIFEST_H
