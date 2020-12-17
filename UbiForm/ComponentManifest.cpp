#include "ComponentManifest.h"
#include <sstream>
#include <memory>

#include "rapidjson/schema.h"

// Constructors
ComponentManifest::ComponentManifest(FILE *jsonFP) {
    // Arbitrary size of read buffer - only changes efficiency of the inputStream constructor
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
    JSON_document.ParseStream(inputStream);

    checkParse();
    fillSchemaMaps();
};

ComponentManifest::ComponentManifest(const char *jsonString) {
    rapidjson::StringStream stream(jsonString);
    JSON_document.ParseStream(stream);

    checkParse();
    fillSchemaMaps();
};

// Check if we have parsed our manifest okay
void ComponentManifest::checkParse(){
    if (JSON_document.HasParseError()){
        std::ostringstream error;
        error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
        error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
        throw std::logic_error(error.str());
    }
    if (!(JSON_document.HasMember("schemas") && JSON_document["schemas"].IsObject())) {
        std::ostringstream error;
        error << "Error, no schemas found defined for the manifest" << std::endl;
        throw std::logic_error(error.str());
    }
}


void ComponentManifest::fillSchemaMaps() {
    assert(JSON_document["schemas"].IsObject());
    for (auto &m : JSON_document["schemas"].GetObject()){
        if (m.value.IsObject() && m.value.HasMember("send")){
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(m.value["send"]);
            auto p1 = std::make_pair(std::string(m.name.GetString()), endpointSchema);
            senderSchemas.insert(p1);
        }
        if (m.value.IsObject() && m.value.HasMember("receive")){
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(m.value["receive"]);
            auto p1 = std::make_pair(std::string(m.name.GetString()), endpointSchema);
            receiverSchemas.insert(p1);
        }
    }
}

// Return the name of the Component
std::string ComponentManifest::getName() {
    assert(JSON_document.HasMember("name"));
    assert(JSON_document["name"].IsString());

    return JSON_document["name"].GetString();
}

SocketMessage *ComponentManifest::getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema) {
    const auto & schemas = JSON_document["schemas"].GetObject();
    if (schemas.HasMember(typeOfEndpoint) && schemas[typeOfEndpoint].IsObject()){
        if(receiveSchema){
            if (schemas[typeOfEndpoint].GetObject().HasMember("receive")){
                return new SocketMessage(schemas[typeOfEndpoint].GetObject()["receive"]);
            }else{
                throw std::logic_error("The endpoint has no receive member");
            }
        }else{
            if (schemas[typeOfEndpoint].GetObject().HasMember("send")){
                return new SocketMessage(schemas[typeOfEndpoint].GetObject()["send"]);
            }else{
                throw std::logic_error("The endpoint has no send member");
            }
        }
    }else{
        throw std::logic_error("No endpoint of that type found");
    }
}

std::shared_ptr<EndpointSchema> ComponentManifest::getReceiverSchema(const std::string &typeOfEndpoint) {
    try{
        return receiverSchemas.at(typeOfEndpoint);
    } catch (std::out_of_range &e) {
        // Explicit rethrow of the exception
        throw;
    }
}

std::shared_ptr<EndpointSchema> ComponentManifest::getSenderSchema(const std::string &typeOfEndpoint) {
    try{
        return senderSchemas.at(typeOfEndpoint);
    } catch (std::out_of_range &e) {
        // Explicit rethrow of the exception
        throw;
    }
}


ComponentManifest::~ComponentManifest()= default;

