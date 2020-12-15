#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include <iostream>
#include <sstream>
#include <vector>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "general_functions.h"


// Note that we need RAPIDJSON_HAS_STDSTRING turned on as we need std string usage
class SocketMessage {
    friend class EndpointSchema;

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

    // This is used to create a new SocketMessage from out inputObject and inputObject is set to null
    SocketMessage(rapidjson::Value &inputObject){
        JSON_document.SetNull();
        swap(JSON_document, inputObject);
    }

public:
    // Base constructor creates an empty object
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
    // Note that this has to be defined in the header as templates are compiled here
    template <class T>
    void addMember(const std::string &attributeName, std::vector<T> inputArray){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (T item : inputArray){
            rapidjson::Value v(item);
            valueArray.PushBack(v, JSON_document.GetAllocator());
        }
        addOrSwap(key,valueArray);
    }

    // Add an array of strings
    void addMember(const std::string &attributeName, std::vector<std::string> inputArray){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (std::string item : inputArray){
            rapidjson::Value v(item,JSON_document.GetAllocator());
            valueArray.PushBack(v, JSON_document.GetAllocator());
        }
        addOrSwap(key,valueArray);
    }

    // Add an array of SocketMessages
    void addMember(const std::string &attributeName, std::vector<SocketMessage*> inputArray){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (auto object : inputArray){
            valueArray.PushBack(object->JSON_document, JSON_document.GetAllocator());
        }
        addOrSwap(key,valueArray);
    }

    // Add a new object - this will set the socketMessage to be zero - MOVE CONSTRUCTOR
    void addMember(const std::string &attributeName, SocketMessage &socketMessage){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        addOrSwap(key, socketMessage.JSON_document.Move());
    }

    void setNull(const std::string &attributeName){
        rapidjson::Value v;
        v.SetNull();
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        addOrSwap(key, v);
    }

    int getInteger(const std::string &attributeName);
    bool getBoolean(const std::string &attributeName);
    std::string getString(const std::string &attributeName);
    // Note that this will set the attributeName to null and return a new POINTER which will need memory handling
    SocketMessage *getObject(const std::string &attributeName);

    template <class T>
    std::vector<T> getArray(const std::string &attributeName);

    bool isNull(const std::string &attributeName){
        return JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsNull();
    }


    std::string stringify() { return stringifyDocument(JSON_document); };

    ~SocketMessage();
};


#endif //UBIFORM_SOCKETMESSAGE_H
