#include "EndpointSchema.h"
#include "../SocketMessage.h"
#include "../general_functions.h"


// Validate a socket message against the manifest
void EndpointSchema::validate(const SocketMessage &messageToValidate) {

    rapidjson::SchemaValidator validator(schema);
    if (!messageToValidate.JSON_document.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Raise exception
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);

        std::ostringstream errorText;
        errorText << "Invalid schema property: " << sb.GetString() << std::endl ;
        errorText << "Invalid keyword: " <<  validator.GetInvalidSchemaKeyword() ;
        throw ValidationError(errorText.str());
    }
}