#include "ComponentManifest.h"


// Constructors
ComponentManifest::ComponentManifest(FILE *jsonFP,SystemSchemas &ss): systemSchemas(ss) {
    // Arbitrary size of read buffer - only changes efficiency of the inputStream constructor
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
    JSON_document.ParseStream(inputStream);

    checkParse();
    fillSchemaMaps();
}

ComponentManifest::ComponentManifest(const char *jsonString, SystemSchemas &ss) : systemSchemas(ss) {
    rapidjson::StringStream stream(jsonString);
    JSON_document.ParseStream(stream);

    checkParse();
    fillSchemaMaps();
}
ComponentManifest::ComponentManifest(SocketMessage *sm, SystemSchemas &ss) : systemSchemas(ss) {
    JSON_document.CopyFrom(sm->JSON_document, JSON_document.GetAllocator());

    checkParse();
    fillSchemaMaps();
}

// Check if we have parsed our manifest okay
// Throws ParseError for parsing issues and ValidationError when manifest doesn't line up
void ComponentManifest::checkParse(){
    if (JSON_document.HasParseError()){
        std::ostringstream error;
        error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
        error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
        throw ParsingError(error.str());
    }
    systemSchemas.getSystemSchema(SystemSchemaName::componentManifest).validate(JSON_document);
}


void ComponentManifest::fillSchemaMaps() {
    assert(JSON_document["schemas"].IsObject());
    for (auto &m : JSON_document["schemas"].GetObject()){
        if (m.value.IsObject() && m.value.HasMember("send")){
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(&m.value["send"], JSON_document.GetAllocator());
            auto p1 = std::make_pair(std::string(m.name.GetString()), endpointSchema);
            senderSchemas.insert(p1);
        }
        if (m.value.IsObject() && m.value.HasMember("receive")){
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(&m.value["receive"], JSON_document.GetAllocator());
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
    if (receiveSchema){
        try{
            return getReceiverSchema(typeOfEndpoint)->getSchemaObject();
        }catch(std::out_of_range &e){
            throw AccessError("No receive endpoint of that type found");
        }
    }else{
        try{
            return getSenderSchema(typeOfEndpoint)->getSchemaObject();
        }catch(std::out_of_range &e){
            throw AccessError("No sender endpoint of that type found");
        }
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

std::string ComponentManifest::getSocketType(const std::string& endpointType) {
    const auto & schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())){
        throw AccessError("No endpoint of type: " + endpointType + " in manifest");
    }

    // Note we know that socketType exists due to the schemas
    std::string socketType = schemas[endpointType].GetObject()["socketType"].GetString();
    return socketType;
}

void ComponentManifest::addSchema(SocketType socketType, const std::string &typeOfEndpoint,
                                  std::shared_ptr<EndpointSchema> receiveSchema,
                                  std::shared_ptr<EndpointSchema> sendSchema) {
    auto schemas = JSON_document["schemas"].GetObject();

    rapidjson::Value newEndpoint(rapidjson::kObjectType);
    newEndpoint.AddMember(rapidjson::Value("socketType", JSON_document.GetAllocator()),
                          rapidjson::Value(convertSocketType(socketType), JSON_document.GetAllocator()),
                          JSON_document.GetAllocator());

    if (socketType == SocketType::Pair || socketType == SocketType::Subscriber) {
        if(receiveSchema == nullptr){
            throw std::logic_error("No receive Schema given");
        }
        rapidjson::Value jsonReceiveSchema(rapidjson::kObjectType);
        jsonReceiveSchema.CopyFrom(*(receiveSchema->JSON_rep), JSON_document.GetAllocator());
        newEndpoint.AddMember(rapidjson::Value("receive", JSON_document.GetAllocator()),
                              jsonReceiveSchema, JSON_document.GetAllocator());

        // Adds with replacement
        receiverSchemas[typeOfEndpoint] = receiveSchema;
    }

    if (socketType == SocketType::Pair || socketType == SocketType::Publisher) {
        if(sendSchema == nullptr){
            throw std::logic_error("No send Schema given");
        }
        rapidjson::Value jsonSendSchema(rapidjson::kObjectType);
        jsonSendSchema.CopyFrom(*(sendSchema->JSON_rep), JSON_document.GetAllocator());
        newEndpoint.AddMember(rapidjson::Value("send", JSON_document.GetAllocator()),
                              jsonSendSchema, JSON_document.GetAllocator());

        // Adds with replacement
        senderSchemas[typeOfEndpoint] = sendSchema;
    }

    if (schemas.HasMember(typeOfEndpoint)){
        schemas.RemoveMember(typeOfEndpoint);
    }
    schemas.AddMember(rapidjson::Value(typeOfEndpoint,JSON_document.GetAllocator()), newEndpoint.Move(),
                      JSON_document.GetAllocator());
}




ComponentManifest::~ComponentManifest()= default;

