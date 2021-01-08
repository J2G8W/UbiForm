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

std::string convertSocketType(SocketType st) {
    switch(st){
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
