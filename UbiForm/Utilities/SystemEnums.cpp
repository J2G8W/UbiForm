#include <stdexcept>
#include "../../include/UbiForm/Utilities/SystemEnums.h"

std::string convertValueType(ValueType vt) {
    switch (vt) {
        case Number:
            return "number";
        case String:
            return "string";
        case Boolean:
            return "boolean";
        case Object:
            return "object";
        case Array:
            return "array";
        case Null:
            return "null";
    }
    throw std::logic_error("Not a Valid value type");
}

std::string convertFromConnectionParadigm(ConnectionParadigm st) {
    switch (st) {
        case Pair:
            return PAIR;
        case Publisher:
            return PUBLISHER;
        case Subscriber:
            return SUBSCRIBER;
        case Reply:
            return REPLY;
        case Request:
            return REQUEST;
    }
    return "Not Valid Socket Type";
}

ConnectionParadigm convertToConnectionParadigm(const std::string &st) {
    if (st == PAIR) {
        return ConnectionParadigm::Pair;
    } else if (st == PUBLISHER) {
        return ConnectionParadigm::Publisher;
    } else if (st == SUBSCRIBER) {
        return ConnectionParadigm::Subscriber;
    } else if (st == REPLY) {
        return ConnectionParadigm::Reply;
    } else if (st == REQUEST) {
        return ConnectionParadigm::Request;
    } else {
        throw std::logic_error("No socket type corresponds to " + st);
    }
}

std::string convertEndpointState(EndpointState es) {
    switch (es) {
        case Invalid:
            return "Invalid";
        case Closed:
            return "Closed";
        case Open:
            return "Open";
        case Dialed:
            return "Dialed";
        case Listening:
            return "Listening";
    }
    return "Not Valid endpoint state";
}
