#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include <iostream>
#include <sstream>
#include <vector>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "general_functions.h"


// Note that we need RAPIDJSON_HAS_STDSTRING turned on
class SocketMessage {
    friend class ComponentManifest;

private:
    rapidjson::Document JSON_document;

    // Takes in a key, value pair (of special type) and either replace the member, or add it
    void addOrSwap(rapidjson::Value &key, rapidjson::Value &valueContainer) {
        if (JSON_document.HasMember(key)) {
            JSON_document[key] = valueContainer;
        } else {
            JSON_document.AddMember(key, valueContainer, JSON_document.GetAllocator());
        }
    }

public:
    SocketMessage() : JSON_document() {
        JSON_document.SetObject();
    };

    explicit SocketMessage(const char *jsonString) {
        rapidjson::StringStream stream(jsonString);
        JSON_document.ParseStream(stream);
        if (JSON_document.HasParseError()){
            std::ostringstream error;
            error << "Error parsing manifest, offset: " << JSON_document.GetErrorOffset();
            error << " , error: " << rapidjson::GetParseError_En(JSON_document.GetParseError()) << std::endl;
            throw std::logic_error(error.str());
        }
    }

    // Add a string value
    void addMember(const std::string &attributeName, const std::string &value) {
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value valueContainer(value, JSON_document.GetAllocator());
        addOrSwap(key, valueContainer);
    }

    // Add an integer value
    void addMember(const std::string &attributeName, int value) {
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value valueContainer(value);
        addOrSwap(key, valueContainer);
    }

    // Add a boolean value
    void addMember(const std::string &attributeName, bool value) {
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value valueContainer(value);
        addOrSwap(key, valueContainer);
    }

    // Add an array
    // Note that this has to be defined in the header as template are compiled as required
    template <class T>
    void addMember(const std::string &attributeName, std::vector<T> inputArray){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (auto item : inputArray){
            valueArray.PushBack(item, JSON_document.GetAllocator());
        }
        addOrSwap(key,valueArray);
    }


    std::string stringify() { return stringifyDocument(JSON_document); };

    ~SocketMessage();
};


#endif //UBIFORM_SOCKETMESSAGE_H
