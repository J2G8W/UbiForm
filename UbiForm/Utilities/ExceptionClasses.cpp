#include "../../include/UbiForm/Utilities/ExceptionClasses.h"
#include "UtilityFunctions.h"

EndpointOpenError::EndpointOpenError(const std::string &error, ConnectionParadigm connectionParadigm,
                                     const std::string &endpointId)
        : std::logic_error(
        "Connection: " + convertFromConnectionParadigm(connectionParadigm) + "\nEndpoint ID: " + endpointId + "\nError: " +
        error) {}

RemoteError::RemoteError(const std::string &errorMsg, const std::string &remoteUrl)
        : std::logic_error("Connecting to " + remoteUrl + "\nError Msg " + errorMsg) {}

NngError::NngError(int rv, const std::string &function_name) :
        std::logic_error("NNG error with function: " + function_name + "\nError text: " +
                         nng_strerror(rv)) { errorCode = rv; }

ValidationError::ValidationError(const std::string &error) : std::logic_error(error) {}

AccessError::AccessError(const std::string &error) : std::logic_error(error) {}

ParsingError::ParsingError(const std::string &error) : std::logic_error(error) {}
