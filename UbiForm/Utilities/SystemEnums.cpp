#include <stdexcept>
#include "SystemEnums.h"

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
}

std::string convertFromSocketType(SocketType st) {
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
}

SocketType convertToSocketType(const std::string &st) {
    if (st == PAIR) {
        return SocketType::Pair;
    } else if (st == PUBLISHER) {
        return SocketType::Publisher;
    } else if (st == SUBSCRIBER) {
        return SocketType::Subscriber;
    } else if (st == REPLY) {
        return SocketType::Reply;
    } else if (st == REQUEST) {
        return SocketType::Request;
    } else {
        throw std::logic_error("No socket type corresponds to " + st);
    }
}
