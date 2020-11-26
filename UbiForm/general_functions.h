#ifndef UBIFORM_GENERALFUNC_H
#define UBIFORM_GENERALFUNC_H

#include <rapidjson/document.h>
#include <nng/nng.h>

void fatal(const char *func, int rv);

std::string stringifyDocument(rapidjson::Document &JSON_document);

class NNG_error : std::logic_error{
public:
    NNG_error(int rv, const std::string& function_name):
        std::logic_error("NNG error with function: " + function_name + "\nError text: " + nng_strerror(rv))
        {}
};

#endif //UBIFORM_GENERALFUNC_H