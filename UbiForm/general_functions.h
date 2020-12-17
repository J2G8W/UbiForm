#ifndef UBIFORM_GENERALFUNC_H
#define UBIFORM_GENERALFUNC_H

#include <rapidjson/document.h>
#include <nng/nng.h>

void fatal(const char *func, int rv);

std::string stringifyDocument(rapidjson::Document &JSON_document);

bool compareSchemaObjects(rapidjson::Value &schema1, rapidjson::Value &schema2);

class NNG_error : public std::logic_error{
public:
    NNG_error(int rv, const std::string& function_name):
        std::logic_error("NNG error with function: " + function_name + "\nError text: " + nng_strerror(rv))
        {}
};

class ValidationError : public std::logic_error{
public:
    explicit ValidationError(const std::string& error) : std::logic_error(error){}
};

class AccessError : public std::logic_error{
public:
    explicit AccessError(const std::string& error) : std::logic_error(error){}
};

#endif //UBIFORM_GENERALFUNC_H