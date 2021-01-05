#ifndef UBIFORM_GENERALFUNC_H
#define UBIFORM_GENERALFUNC_H

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <nng/nng.h>


std::string stringifyValue(rapidjson::Value &JSON_document);

/**
 * @brief Compare whether our objects are "equal" according to the rules defined by Julian, that is, functionally equivalent
 * @param schema1
 * @param schema2
 * @return boolean value for "equality"
 */
bool compareSchemaObjects(rapidjson::Value &schema1, rapidjson::Value &schema2);

bool compareSchemaArrays(rapidjson::Value &schema1, rapidjson::Value &schema2);

/**
 * Represents an error made in nng system.
 * Text of the error uses nng's standard reporting mechanism.
 */
class NngError : public std::logic_error{
public:
    int errorCode;
    NngError(int rv, const std::string& function_name):
        std::logic_error("NNG error with function: " + function_name + "\nError text: " + nng_strerror(rv))
        {errorCode = rv;}
};

/**
 * Represents an error when we validate some object against an EndpointSchema.
 * Text of the error should come directly from rapidjson error generation
 */
class ValidationError : public std::logic_error{
public:
    explicit ValidationError(const std::string& error) : std::logic_error(error){}
};

/**
 * Represents an error when we try to access a member of SocketMessage that isn't valid.
 * Text of the error should come directly from rapidjson error generation
 */
class AccessError : public std::logic_error{
public:
    explicit AccessError(const std::string& error) : std::logic_error(error){}
};

/**
 * Represents an error when we parse some JSON object.
 * Text of the error should come directly from rapidjson error generation
 */
class ParsingError : public std::logic_error{
public:
    explicit ParsingError(const std::string& error) : std::logic_error(error){}
};

class SocketOpenError : public std::logic_error{
public:
    explicit SocketOpenError(const std::string & error) : std::logic_error(error){}
};

// USE ONLY FOR TESTING PURPOSES
rapidjson::Document* parseFromFile(const char * address);

#endif //UBIFORM_GENERALFUNC_H