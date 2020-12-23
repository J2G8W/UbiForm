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
    /**
     * Create an empty socket message that we can add to
     */
    SocketMessage() : JSON_document() {
        JSON_document.SetObject();
    };

    /**
     * Create socket message from string input, largely for use from networks
     * @param jsonString - string input
     */
    explicit SocketMessage(const char *jsonString);

    ///@{
    /**
     * @name addMember
     * These methods add or change attributes in the SocketMessage, we don't allow repeated attribute names (it overwrites)
     */
    void addMember(const std::string &attributeName, const std::string &value);
    void addMember(const std::string &attributeName, const char * value);
    void addMember(const std::string &attributeName, int value);
    void addMember(const std::string &attributeName, bool value);

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

    void addMember(const std::string &attributeName, const std::vector<std::string>& inputArray);

    /// @brief Copy constructs the SocketMessages
    // TODO - No copy constructing
    void addMember(const std::string &attributeName, const std::vector<SocketMessage*>& inputArray);
    /// @brief Copy constructs the given SocketMessage
    // TODO - make this more efficient by getting rid of copy and figuring out relevant MemoryPool semantics
    void addMember(const std::string &attributeName, SocketMessage &socketMessage);

    void setNull(const std::string &attributeName);
    ///@}



    ///@{
    /**
     * @name Getters from the SocketMessage
     */
    int getInteger(const std::string &attributeName);
    bool getBoolean(const std::string &attributeName);
    std::string getString(const std::string &attributeName);
    /// @brief This uses copy constructing
    SocketMessage *getObject(const std::string &attributeName);

    // TODO - sort documentation for this (looks wierd)
    template <class T>
    std::vector<T> getArray(const std::string &attributeName);

    bool isNull(const std::string &attributeName){
        return JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsNull();
    }
    ///@}

    std::string stringify() { return stringifyDocument(JSON_document); };

    ~SocketMessage();
};


#endif //UBIFORM_SOCKETMESSAGE_H
