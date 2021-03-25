#ifndef UBIFORM_GENERALFUNC_H
#define UBIFORM_GENERALFUNC_H

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <nng/nng.h>
#include "../../include/UbiForm/Utilities/SystemEnums.h"
#include "../../include/UbiForm/Utilities/ExceptionClasses.h"


std::string stringifyValue(rapidjson::Value &JSON_document);

/**
 * @brief Compare whether our objects are "equal" according to the rules defined by Julian, that is, functionally equivalent
 * @param schema1
 * @param schema2
 * @return boolean value for "equality"
 */
bool compareSchemaObjects(rapidjson::Value &schema1, rapidjson::Value &schema2);

bool compareSchemaArrays(rapidjson::Value &schema1, rapidjson::Value &schema2);


// USE ONLY FOR TESTING PURPOSES
rapidjson::Document *parseFromFile(const char *address);

#endif //UBIFORM_GENERALFUNC_H