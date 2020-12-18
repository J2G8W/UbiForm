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
#include "endpoints/EndpointSchema.h"
#include "SystemSchemas/SystemSchemas.h"


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
    // Accept JSON input as string
    explicit ComponentManifest(const char *jsonString, SystemSchemas & es);

    // Accept JSON input as a FILE pointer
    // This is used rather than istreams as we get better performance for rapidjson
    explicit ComponentManifest(FILE *jsonFP, SystemSchemas &es);

    // TODO - don't copy, use move constructors - far more efficient
    explicit ComponentManifest(SocketMessage* sm,SystemSchemas &es ) : ComponentManifest(sm->stringify().c_str(), es) {
    }


    std::shared_ptr<EndpointSchema> getReceiverSchema(const std::string& typeOfEndpoint);
    std::shared_ptr<EndpointSchema> getSenderSchema(const std::string& typeOfEndpoint);

    std::string getName();

    std::string getSocketType(std::string endpointType);

    std::string stringify() { return stringifyDocument(JSON_document); };


    SocketMessage * getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema);

    ~ComponentManifest();
};


#endif //UBIFORM_COMPONENTMANIFEST_H
