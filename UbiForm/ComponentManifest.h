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


class ComponentManifest {
private:
    rapidjson::Document JSON_document;

    // TODO -  combine into one map at some point
    // Note that these maps will "auto delete" as we are using shared pointers so we don't need to worry about their memory
    std::map<std::string, std::shared_ptr<EndpointSchema> > receiverSchemas;
    std::map<std::string, std::shared_ptr<EndpointSchema> > senderSchemas;


    void checkParse();
    void fillSchemaMaps();


public:
    // Accept JSON input as string
    explicit ComponentManifest(const char *jsonString);

    // Accept JSON input as a FILE pointer
    // This is used rather than istreams as we get better performance for rapidjson
    explicit ComponentManifest(FILE *jsonFP);



    std::shared_ptr<EndpointSchema> getReceiverSchema(const std::string& typeOfEndpoint){
        try{
            return receiverSchemas.at(typeOfEndpoint);
        } catch (std::out_of_range &e) {
            // Explicit rethrow of the exception
            throw;
        }

    }
    std::shared_ptr<EndpointSchema> getSenderSchema(const std::string& typeOfEndpoint){
        try{
            return senderSchemas.at(typeOfEndpoint);
        } catch (std::out_of_range &e) {
            // Explicit rethrow of the exception
            throw;
        }
    }

    // We return C++ strings such that memory management is simpler
    std::string getName();

    std::string stringify() { return stringifyDocument(JSON_document); };


    ~ComponentManifest();
};


#endif //UBIFORM_COMPONENTMANIFEST_H
