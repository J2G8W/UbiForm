#ifndef UBIFORM_ENDPOINTMESSAGE_H
#define UBIFORM_ENDPOINTMESSAGE_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>


#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/error/en.h"
#include "../../UbiForm/Utilities/UtilityFunctions.h"


/**
 * This class is used to represent all message around the system. It is an abstraction of a JSON object and can be turned
 * into a string for when we are sending. Note this class only work with RAPIDJSON_HAS_STDSTRING turned on
 */
class EndpointMessage {
    friend class EndpointSchema;

    friend class ComponentRepresentation;

    friend class ComponentManifest;

private:
    rapidjson::Document JSON_document;
    std::vector<std::unique_ptr<EndpointMessage>> dependants;

    // Takes in a key, value pair (of special type) and either replace the member, or add it
    void addOrSwap(rapidjson::Value &key, rapidjson::Value &valueContainer) {
        if (JSON_document.HasMember(key)) {
            JSON_document[key] = valueContainer;
        } else {
            JSON_document.AddMember(key, valueContainer, JSON_document.GetAllocator());
        }
    }


    // COPY CONSTRUCTOR - needed for sensible memory allocation!
    EndpointMessage(rapidjson::Value &inputObject, bool copyConstruct = true) {
        if (copyConstruct) {
            JSON_document.SetObject();
            JSON_document.CopyFrom(inputObject, JSON_document.GetAllocator());
        } else {
            JSON_document.SetNull();
            JSON_document.Swap(inputObject);
        }
    }

public:
    /**
     * @brief Create an empty endpoint message that we can add to
     */
    EndpointMessage() : JSON_document() {
        JSON_document.SetObject();
    };

    /**
     * @brief Create endpoint message from string input, largely for use from networks
     * @param jsonString - string input
     */
    explicit EndpointMessage(const char *jsonString);

    /// Returns a string of the EndpointMessage for debugging and sending on wire
    std::string stringify() { return stringifyValue(JSON_document); };

    ///@{
    /**
     * @name Add Member (Copy constructors)
     * These methods add or change attributes in the EndpointMessage, we don't allow repeated attribute names (it overwrites)
     * and all the methods exist to COPY in
     */
    void addMember(const std::string &attributeName, const std::string &value);

    void addMember(const std::string &attributeName, const char *value);

    void addMember(const std::string &attributeName, int value);

    void addMember(const std::string &attributeName, bool value);

    // Note that this has to be defined in the header as templates are compiled here
    template<class T>
    void addMember(const std::string &attributeName, std::vector<T> inputArray) {
        rapidjson::Value key(attributeName, JSON_document.GetAllocator());

        rapidjson::Value valueArray(rapidjson::kArrayType);
        valueArray.Reserve(inputArray.size(), JSON_document.GetAllocator());
        for (T item : inputArray) {
            rapidjson::Value v(item);
            valueArray.PushBack(v, JSON_document.GetAllocator());
        }
        addOrSwap(key, valueArray);
    }

    void addMember(const std::string &attributeName, const std::vector<std::string> &inputArray);

    void addMember(const std::string &attributeName, const std::vector<EndpointMessage *> &inputArray);

    void addMember(const std::string &attributeName, EndpointMessage &endpointMessage);

    void setNull(const std::string &attributeName);
    ///@}


    ///@{
    ///@name Add Member (move constructors)
    /**
     * @brief Moves the value from the the input endpointMessage into us. Once a message is moved in (std::unique_ptr) we have
     * ownership and deltion of it.
     * @param attributeName - name of attribute
     */
    void addMoveObject(const std::string &attributeName, std::unique_ptr<EndpointMessage> endpointMessage);

    void addMoveArrayOfObjects(const std::string &attributeName, std::vector<std::unique_ptr<EndpointMessage>> &inputArray);
    ///@}

    ///@{
    /**
     * @name Getters from the EndpointMessage (Copy Constructor)
     * @param attributeName - The attribute we want
     * @throws AccessError - The desired attribute is not in the EndpointMessage or is the wrong type
     */
    int getInteger(const std::string &attributeName);

    bool getBoolean(const std::string &attributeName);

    std::string getString(const std::string &attributeName);

    std::unique_ptr<EndpointMessage> getCopyObject(const std::string &attributeName);

    bool isNull(const std::string &attributeName);

    template<class T>
    std::vector<T> getArray(const std::string &attributeName);
    ///@}

    ///@{
    ///@name - Getters from EndpointMessage (move rather than copy)
    /**
     * Move  from within the current object. NOTE that the parent must live as long as the new child due to memory handling
     * issues with rapidjson. If this is a problem use copy versions which are less efficient but hasn't got memory issues
     * @param attributeName - Attribute to get
     * @throws AccessError - The desired attribute is not in the EndpointMessage or is the wrong type
     */
    std::unique_ptr<EndpointMessage> getMoveObject(const std::string &attributeName);

    std::vector<std::unique_ptr<EndpointMessage> > getMoveArrayOfObjects(const std::string &attributeName);
    ///@}


    /**
     * @return whether the endpointMessage itself is a null value
     */
    bool isNull() {
        return JSON_document.IsNull();
    }

    /**
     * @brief Get all the properties names that are associated with the messages
     * @return Vector of the property name of the EndpointMessage
     */
    std::vector<std::string> getKeys();

    /**
     * @breif Check if EndpointMessage has property
     * @param attributeName - Attribute name to search on
     * @return Whether the EndpointMessage has an attribute of the given name
     */
    bool hasMember(const std::string &attributeName) {
        return JSON_document.HasMember(attributeName);
    }


    ~EndpointMessage();
};


#endif //UBIFORM_ENDPOINTMESSAGE_H
