#include "ComponentManifest.h"
#include <sstream>

// Constructors
ComponentManifest::ComponentManifest(FILE *jsonFP) {
    // Arbitrary size of read buffer - only changes efficiency of the inputStream constructor
    char readBuffer[65536];
    rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
    JSON_document.ParseStream(inputStream);

    checkParse();

    schema = new rapidjson::SchemaDocument(JSON_document["schema"]);
};

ComponentManifest::ComponentManifest(const char *jsonString) {
    rapidjson::StringStream stream(jsonString);
    JSON_document.ParseStream(stream);

    checkParse();

    schema = new rapidjson::SchemaDocument(JSON_document["schema"]);
};

// Check if we have parsed our manifest okay
void ComponentManifest::checkParse(){
    if (JSON_document.HasParseError()){
        std::ostringstream error;
        error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
        error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
        throw std::logic_error(error.str());
    }
    if (!(JSON_document.HasMember("schema") && JSON_document["schema"].IsObject())) {
        std::ostringstream error;
        error << "Error, no schema found defined for the manifest" << std::endl;
        throw std::logic_error(error.str());
    }
}

// Return the name of the Component
std::string ComponentManifest::getName() {
    assert(JSON_document.HasMember("name"));
    assert(JSON_document["name"].IsString());

    return JSON_document["name"].GetString();
}

// Validate a socket message against the manifest
void ComponentManifest::validate(const SocketMessage &messageToValidate) {

    rapidjson::SchemaValidator validator(*schema);
    if (!messageToValidate.JSON_document.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Raise exception
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);

        std::ostringstream errorText;
        errorText << "Invalid schema: " << sb.GetString() << std::endl;
        errorText << "Invalid keyword: " <<  validator.GetInvalidSchemaKeyword() << std::endl;
        throw std::logic_error(errorText.str());
    }
}