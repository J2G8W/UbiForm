#ifndef UBIFORM_EXCEPTIONCLASSES_H
#define UBIFORM_EXCEPTIONCLASSES_H
#include "SystemEnums.h"
/**
 * Represents an error made in nng system.
 * Text of the error uses nng's standard reporting mechanism.
 */
class NngError : public std::logic_error {
public:
    int errorCode;

    NngError(int rv, const std::string &function_name);
};

/**
 * Represents an error when we validate some object against an EndpointSchema.
 * Text of the error should come directly from rapidjson error generation
 */
class ValidationError : public std::logic_error {
public:
    explicit ValidationError(const std::string &error);
};

/**
 * Represents an error when we try to access a member of EndpointMessage that isn't valid.
 * Text of the error should come directly from rapidjson error generation
 */
class AccessError : public std::logic_error {
public:
    explicit AccessError(const std::string &error);
};

/**
 * Represents an error when we parse some JSON object.
 * Text of the error should come directly from rapidjson error generation
 */
class ParsingError : public std::logic_error {
public:
    explicit ParsingError(const std::string &error);
};

class EndpointOpenError : public std::logic_error {
public:
    EndpointOpenError(const std::string &error, ConnectionParadigm connectionParadigm, const std::string &endpointId);
};

class RemoteError : public std::logic_error {
public:
    RemoteError(const std::string &errorMsg, const std::string &remoteUrl);
};

#endif //UBIFORM_EXCEPTIONCLASSES_H
