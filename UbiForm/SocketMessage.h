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
    friend class ComponentRepresentation;
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


    // COPY CONSTRUCTOR - needed for sensible memory allocation!
    // TODO - Consider making a move constructor
    explicit SocketMessage(rapidjson::Value &inputObject){
        JSON_document.SetObject();
        JSON_document.CopyFrom(inputObject,JSON_document.GetAllocator());
    }

public:
    // Base constructor creates an empty object
    SocketMessage() : JSON_document() {
        JSON_document.SetObject();
    };

    explicit SocketMessage(const char *jsonString);

    // Add a string value
    void addMember(const std::string &attributeName, const std::string &value) {
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value valueContainer(value, JSON_document.GetAllocator());
        addOrSwap(key, valueContainer);
    }

    void addMember(const std::string &attributeName, const char * value){
        addMember(attributeName, std::string(value));
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
    // TODO - No copy constructing
    void addMember(const std::string &attributeName, std::vector<SocketMessage*> inputArray){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (auto object : inputArray){
            rapidjson::Value v;
            v.SetObject();
            v.CopyFrom(object->JSON_document, JSON_document.GetAllocator());
            valueArray.PushBack(v, JSON_document.GetAllocator());
        }
        addOrSwap(key,valueArray);
    }

    // This will COPY the message to become an object in our main document
    // TODO - make this more efficient by getting rid of copy and figuring out relevant MemoryPool
    //  semantics
    void addMember(const std::string &attributeName, SocketMessage &socketMessage){
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());
        rapidjson::Value v;
        v.CopyFrom(socketMessage.JSON_document,JSON_document.GetAllocator());
        addOrSwap(key,v);
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
