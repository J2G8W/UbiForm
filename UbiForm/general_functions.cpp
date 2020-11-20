#include <iostream>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "nng/nng.h"

#include "general_functions.h"

// For the moment we use the standard NNG function to quit
void fatal(const char *func, int rv) {
    std::cerr << "Problem with function: " << func << "\nError text: " << nng_strerror(rv) << "\n";
    exit(1);
}

// TODO - optimise this for speed
char *stringifyDocument(rapidjson::Document &JSON_document) {
    rapidjson::StringBuffer buffer;

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    JSON_document.Accept(writer);

    // We copy the string from the buffer to our return string so that it is not squashed when we return
    // Plus 1 is used to make sure we copy over a null terminator
    int stringLength = buffer.GetLength();
    char *jsonReturnString = new char[stringLength];
    strncpy(jsonReturnString, buffer.GetString(), static_cast<size_t>(stringLength + 1));
    jsonReturnString[stringLength] = '\0';

    return jsonReturnString;
}
