#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include <iostream>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
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

    explicit SocketMessage(char *jsonString) {
        rapidjson::StringStream stream(jsonString);
        JSON_document.ParseStream(stream);
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


    char *stringify() { return stringifyDocument(JSON_document); };

    ~SocketMessage();
};


#endif //UBIFORM_SOCKETMESSAGE_H
