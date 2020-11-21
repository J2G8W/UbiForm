#ifndef UBIFORM_ENDPOINTSCHEMA_H
#define UBIFORM_ENDPOINTSCHEMA_H


#include <rapidjson/schema.h>
#include "../SocketMessage.h"

class EndpointSchema {
private:
    rapidjson::SchemaDocument schema;

public:
    explicit EndpointSchema(rapidjson::Value &doc) : schema(doc) { }
    void validate(const SocketMessage &messageToValidate);
};


#endif //UBIFORM_ENDPOINTSCHEMA_H
