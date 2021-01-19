#include "ComponentManifest.h"


// Constructors
ComponentManifest::ComponentManifest(FILE *jsonFP, SystemSchemas &ss) : systemSchemas(ss) {
    setManifest(jsonFP);
}

ComponentManifest::ComponentManifest(const char *jsonString, SystemSchemas &ss) : systemSchemas(ss) {
    setManifest(jsonString);
}

ComponentManifest::ComponentManifest(SocketMessage *sm, SystemSchemas &ss) : systemSchemas(ss) {
    setManifest(sm);
}


void ComponentManifest::setManifest(FILE *jsonFP) {
    // Arbitrary size of read buffer - only changes efficiency of the inputStream constructor
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
    JSON_document.ParseStream(inputStream);

    checkParse();
    fillSchemaMaps();
}

void ComponentManifest::setManifest(const char *jsonString) {
    rapidjson::StringStream stream(jsonString);
    JSON_document.ParseStream(stream);

    checkParse();
    fillSchemaMaps();
}

void ComponentManifest::setManifest(SocketMessage *sm) {
    JSON_document.CopyFrom(sm->JSON_document, JSON_document.GetAllocator());

    checkParse();
    fillSchemaMaps();
}


// Check if we have parsed our manifest okay
// Throws ParseError for parsing issues and ValidationError when manifest doesn't line up
void ComponentManifest::checkParse() {
    if (JSON_document.HasParseError()) {
        std::ostringstream error;
        error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
        error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
        throw ParsingError(error.str());
    }
    systemSchemas.getSystemSchema(SystemSchemaName::componentManifest).validate(JSON_document);
}


void ComponentManifest::fillSchemaMaps() {
    assert(JSON_document["schemas"].IsObject());
    for (auto &m : JSON_document["schemas"].GetObject()) {
        if (m.value.IsObject() && m.value.HasMember("send")) {
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(&m.value["send"],
                                                                                              JSON_document.GetAllocator());
            senderSchemas[std::string(m.name.GetString())] = endpointSchema;
        }
        if (m.value.IsObject() && m.value.HasMember("receive")) {
            std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(&m.value["receive"],
                                                                                              JSON_document.GetAllocator());
            receiverSchemas[std::string(m.name.GetString())] = endpointSchema;
        }
    }
}

// Return the name of the Component
std::string ComponentManifest::getName() {
    return getProperty("name");
}

void ComponentManifest::setName(const std::string &name) {
    setProperty("name", name);
}

std::unique_ptr<SocketMessage>
ComponentManifest::getSchemaObject(const std::string &typeOfEndpoint, bool receiveSchema) {
    if (receiveSchema) {
        try {
            return getReceiverSchema(typeOfEndpoint)->getSchemaObject();
        } catch (std::out_of_range &e) {
            throw AccessError("No receive endpoint of that type found");
        }
    } else {
        try {
            return getSenderSchema(typeOfEndpoint)->getSchemaObject();
        } catch (std::out_of_range &e) {
            throw AccessError("No sender endpoint of that type found");
        }
    }
}

std::shared_ptr<EndpointSchema> ComponentManifest::getReceiverSchema(const std::string &typeOfEndpoint) {
    try {
        return receiverSchemas.at(typeOfEndpoint);
    } catch (std::out_of_range &e) {
        // Explicit rethrow of the exception
        throw;
    }
}

std::shared_ptr<EndpointSchema> ComponentManifest::getSenderSchema(const std::string &typeOfEndpoint) {
    try {
        return senderSchemas.at(typeOfEndpoint);
    } catch (std::out_of_range &e) {
        // Explicit rethrow of the exception
        throw;
    }
}

std::string ComponentManifest::getSocketType(const std::string &endpointType) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())) {
        throw AccessError("No endpoint of type: " + endpointType + " in manifest");
    }

    // Note we know that socketType exists due to the schemas
    std::string socketType = schemas[endpointType].GetObject()["socketType"].GetString();
    return socketType;
}

void ComponentManifest::addEndpoint(SocketType socketType, const std::string &typeOfEndpoint,
                                    std::shared_ptr<EndpointSchema> receiveSchema,
                                    std::shared_ptr<EndpointSchema> sendSchema) {
    auto schemas = JSON_document["schemas"].GetObject();

    rapidjson::Value newEndpoint(rapidjson::kObjectType);
    newEndpoint.AddMember(rapidjson::Value("socketType", JSON_document.GetAllocator()),
                          rapidjson::Value(convertFromSocketType(socketType), JSON_document.GetAllocator()),
                          JSON_document.GetAllocator());

    if (socketType == SocketType::Pair || socketType == SocketType::Subscriber) {
        if (receiveSchema == nullptr) {
            throw std::logic_error("No receive Schema given");
        }
        rapidjson::Value jsonReceiveSchema(rapidjson::kObjectType);
        jsonReceiveSchema.CopyFrom(*(receiveSchema->JSON_rep), JSON_document.GetAllocator());
        newEndpoint.AddMember(rapidjson::Value("receive", JSON_document.GetAllocator()),
                              jsonReceiveSchema, JSON_document.GetAllocator());

    }

    if (socketType == SocketType::Pair || socketType == SocketType::Publisher) {
        if (sendSchema == nullptr) {
            throw std::logic_error("No send Schema given");
        }
        rapidjson::Value jsonSendSchema(rapidjson::kObjectType);
        jsonSendSchema.CopyFrom(*(sendSchema->JSON_rep), JSON_document.GetAllocator());
        newEndpoint.AddMember(rapidjson::Value("send", JSON_document.GetAllocator()),
                              jsonSendSchema, JSON_document.GetAllocator());

        // Adds with replacement
        senderSchemas[typeOfEndpoint] = sendSchema;
    }

    if (schemas.HasMember(typeOfEndpoint)) {
        schemas.RemoveMember(typeOfEndpoint);
    }
    schemas.AddMember(rapidjson::Value(typeOfEndpoint, JSON_document.GetAllocator()), newEndpoint.Move(),
                      JSON_document.GetAllocator());


    if (socketType == SocketType::Pair || socketType == SocketType::Subscriber) {
        std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(
                &(schemas[typeOfEndpoint].GetObject()["receive"]),
                JSON_document.GetAllocator());
        receiverSchemas[typeOfEndpoint] = endpointSchema;
    }
    if (socketType == SocketType::Pair || socketType == SocketType::Publisher) {
        std::shared_ptr<EndpointSchema> endpointSchema = std::make_shared<EndpointSchema>(
                &(schemas[typeOfEndpoint].GetObject()["send"]),
                JSON_document.GetAllocator());
        senderSchemas[typeOfEndpoint] = endpointSchema;
    }
}

void ComponentManifest::setProperty(const std::string &propertyName, const std::string &value) {
    if (propertyName == "urls" || propertyName == "port" || propertyName == "schemas") {
        throw AccessError(propertyName + " is a reserved name");
    }
    if (JSON_document.HasMember(propertyName)) {
        JSON_document[propertyName] = rapidjson::Value(value, JSON_document.GetAllocator());
    } else {
        JSON_document.AddMember(rapidjson::Value(propertyName, JSON_document.GetAllocator()),
                                rapidjson::Value(value, JSON_document.GetAllocator()),
                                JSON_document.GetAllocator());
    }
}

std::string ComponentManifest::getProperty(const std::string &propertyName) {
    if (JSON_document.HasMember(propertyName) && JSON_document[propertyName].IsString()) {
        return JSON_document[propertyName].GetString();
    } else {
        throw AccessError("Manifest does not have string property " + propertyName + "\n" + stringify());
    }
}

void ComponentManifest::removeProperty(const std::string &propertyName) {
    if (propertyName == "urls" || propertyName == "port" || propertyName == "schemas" || propertyName == "name") {
        throw AccessError(propertyName + " is a reserved name");
    }
    if (JSON_document.HasMember(propertyName)) {
        JSON_document.EraseMember(propertyName);
    }
}

bool ComponentManifest::hasProperty(const std::string &property) {
    return JSON_document.HasMember(property) && JSON_document[property].IsString();
}

std::vector<std::string> ComponentManifest::getAllEndpointTypes() {
    std::vector<std::string> endpointTypes;
    for (auto &m: JSON_document["schemas"].GetObject()) {
        endpointTypes.emplace_back(m.name.GetString());
    }
    return endpointTypes;
}

void ComponentManifest::addListenPort(const std::string &endpointType, int port) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())) {
        throw AccessError("No endpoint of type: " + endpointType + " in manifest");
    }
    std::string socketType = std::string(schemas[endpointType].GetObject()["socketType"].GetString());
    if (!(socketType == PUBLISHER || socketType == REPLY)) {
        throw AccessError(endpointType + " is not valid to add listener port");
    }
    if (schemas[endpointType].HasMember("listenPort")) {
        schemas[endpointType].GetObject()["listenPort"] = rapidjson::Value(port);
    } else {
        schemas[endpointType].AddMember("listenPort", port, JSON_document.GetAllocator());
    }
}

bool ComponentManifest::hasListenPort(const std::string &endpointType) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())) {
        return false;
    }
    return schemas[endpointType].GetObject().HasMember("listenPort");
}

int ComponentManifest::getListenPort(const std::string &endpointType) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())) {
        throw AccessError("No endpoint of type: " + endpointType + " in manifest");
    }
    if (!schemas[endpointType].GetObject().HasMember("listenPort")) {
        throw AccessError(endpointType + " has no valid listenPort");
    }
    return schemas[endpointType].GetObject()["listenPort"].GetInt();
}

void ComponentManifest::removeListenPort(const std::string &endpointType) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (!(schemas.HasMember(endpointType) && schemas[endpointType].IsObject())) {
        throw AccessError("No endpoint of type: " + endpointType + " in manifest");
    }
    if (schemas[endpointType].GetObject().HasMember("listenPort")) {
        schemas[endpointType].GetObject().EraseMember("listenPort");
    }
}


ComponentManifest::~ComponentManifest() = default;

