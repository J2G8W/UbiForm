#ifndef UBIFORM_SOCKETMESSAGE_H
#define UBIFORM_SOCKETMESSAGE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "Utilities/UtilityFunctions.h"


// Note that we need RAPIDJSON_HAS_STDSTRING turned on as we need std string usage
class SocketMessage {
    friend class EndpointSchema;
    friend class ComponentRepresentation;
    friend class ComponentManifest;

private:
    rapidjson::Document JSON_document;
    std::vector<std::unique_ptr<SocketMessage>> dependants;

    // Takes in a key, value pair (of special type) and either replace the member, or add it
    void addOrSwap(rapidjson::Value &key, rapidjson::Value &valueContainer) {
        if (JSON_document.HasMember(key)) {
            JSON_document[key] = valueContainer;
        } else {
            JSON_document.AddMember(key, valueContainer, JSON_document.GetAllocator());
        }
    }


    // COPY CONSTRUCTOR - needed for sensible memory allocation!
    SocketMessage(rapidjson::Value &inputObject, bool copyConstruct=true) {
        if (copyConstruct) {
            JSON_document.SetObject();
            JSON_document.CopyFrom(inputObject, JSON_document.GetAllocator());
        }else{
            JSON_document.SetNull();
            JSON_document.Swap(inputObject);
        }
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
    void addMember(const std::string &attributeName, SocketMessage &socketMessage);

    void setNull(const std::string &attributeName);
    ///@}

    /**
     * @brief Moves the value from the the input socketMessage into us, use unique_ptr such that users can't mess with this
     * @param attributeName - name of attribute
     * @param socketMessage - thing to be moved into us
     */
    void addMoveObject(const std::string &attributeName, std::unique_ptr<SocketMessage> socketMessage);

    ///@{
    /**
     * @name Getters from the SocketMessage
     */
    int getInteger(const std::string &attributeName);
    bool getBoolean(const std::string &attributeName);
    std::string getString(const std::string &attributeName);
    /// @brief This uses copy constructing
    std::unique_ptr<SocketMessage> getCopyObject(const std::string &attributeName);

    template <class T>
    std::vector<T> getArray(const std::string &attributeName);

    bool isNull(const std::string &attributeName){
        return JSON_document.HasMember(attributeName) && JSON_document[attributeName].IsNull();
    }
    ///@}

    /**
     * Move an object from within the current object. NOTE that the parent must live as long as the new child due to memory handling
     * issues with rapidjson. If this is a problem use getCopyObject which is less efficient but hasn't got memory issues
     * @param attributeName - Attribute to get
     * @return std::unique_ptr to a SocketMessage which is the sub-object desired, uses special pointers to make CLEAR that
     * these should have short life spans
     */
    std::unique_ptr<SocketMessage> getMoveObject(const std::string &attributeName);
    /**
     * Move an array of objects from within the current object. NOTE that the parent must live as long as the new child due to memory handling
     * issues with rapidjson. If this is a problem use getArray which is less efficient but hasn't got memory issues
     * @param attributeName - Attibutre to get
     * @return vector of std::unique_ptr's for ease of memory handling
     */
    std::vector<std::unique_ptr<SocketMessage> > getMoveArrayOfObjects(const std::string &attributeName);

    /**
     * @return whether the socketMessage itself is a null value
     */
    bool isNull(){
        return JSON_document.IsNull();
    }


    std::vector<std::string> getKeys();

    /// Returns a string of the SocketMessage for debugging and sending on wire
    std::string stringify() { return stringifyValue(JSON_document); };

    ~SocketMessage();
};


#endif //UBIFORM_SOCKETMESSAGE_H
