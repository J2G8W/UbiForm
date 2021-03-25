#ifndef UBIFORM_SYSTEMENUMS_H
#define UBIFORM_SYSTEMENUMS_H


#include <string>
#include <memory>

// NOTE THAT these strings are defined in the various schemas in SystemSchemas

#define PAIR "pair"
#define PUBLISHER "publisher"
#define SUBSCRIBER "subscriber"
#define REPLY "reply"
#define REQUEST "request"

/// Describes the different connection paradigms we can have
enum ConnectionParadigm {
    Pair, Publisher, Subscriber, Reply, Request
};

std::string convertFromConnectionParadigm(ConnectionParadigm st);

ConnectionParadigm convertToConnectionParadigm(const std::string &st);

/// Describes the different types in our endpoint schemas/endpoint messages
enum ValueType {
    Number, String, Boolean, Object, Array, Null
};

std::string convertValueType(ValueType vt);

/// Respresent the System Schemas
enum SystemSchemaName {
    componentManifest, endpointCreationRequest, endpointCreationResponse,
    additionRequest, additionResponse, byIdRequest, byIdResponse,
    bySchemaRequest, bySchemaResponse, componentIdsRequest, componentIdsResponse, updateRequest,
    generalRDRequest, generalRDResponse, generalEndpointRequest, generalEndpointResponse
};

/// Used on Component to describe how it has been loaded in
enum ConnectionType {
    IPC, TCP, LocalTCP
};

/// Represents the state an endpoint is in
enum EndpointState {
    Invalid, Closed, Open, Dialed, Listening
};

std::string convertEndpointState(EndpointState es);

class Endpoint;
class PairEndpoint;
class EndpointMessage;
/// This represents the type of functions which can be called on Endpoint startup. We give a pointer to the endpoint which
/// has been started and a pointer to some arbitrary data structure which the developer can specify
typedef void (*endpointStartupFunction)(Endpoint*, void *);

typedef void (*endOfStreamCallback)(PairEndpoint*, void*);

typedef void (*endpointAdditionCallBack)(std::string, void*);

typedef void (*receiveMessageCallBack)(EndpointMessage *, void *);


// Strings used for Background Requester and Listener
#define BACKGROUND_CREATE_AND_LISTEN "req_conn"
#define BACKGROUND_CREATE_AND_DIAL "req_dial"
#define BACKGROUND_REQUEST_ENDPOINT_INFO "endpoint_info"

#define BACKGROUND_ADD_RDH "add_rdh"
#define BACKGROUND_REMOVE_RDH "remove_rdh"

#define BACKGROUND_CREATE_RDH "create_rdh"
#define BACKGROUND_CLOSE_RDH "close_rdh"

#define BACKGROUND_GET_LOCATIONS_OF_RDH "locations_rdh"

#define BACKGROUND_3RD_PARTY_REMOTE_LISTEN_THEN_DIAL "tell_req_conn"
#define BACKGROUND_3RD_PARTY_LOCAL_LISTEN_THEN_REMOTE_DIAL "3_local_listen"

#define BACKGROUND_CHANGE_ENDPOINT_SCHEMA "add_endpoint_schema"
#define BACKGROUND_CHANGE_MANIFEST "change_manifest"
#define BACKGROUND_GET_COMPONENT_MANIFEST "get_manifest"

#define BACKGROUND_CLOSE_ENDPOINTS "close_endpoints"
#define BACKGROUND_CLOSE_ENDPOINT_BY_ID "close_endpoint_id"




// Strings used for Resource Discovery comms
// NOTE THAT THESE ARE ALSO DEFINED IN "SystemSchemas/resource_discovery_*_request.json"
#define RESOURCE_DISCOVERY_ADD_COMPONENT "addition"
#define RESOURCE_DISCOVERY_DEREGISTER_COMPONENT "deRegister"
#define RESOURCE_DISCOVERY_UPDATE_MANIFEST "update"
#define RESOURCE_DISCOVERY_NOTIFY_ENDPOINT_PORT_LISTEN "endpointPortListen"

#define RESOURCE_DISCOVERY_REQUEST_BY_ID "requestId"
#define RESOURCE_DISCOVERY_REQUEST_BY_SCHEMA "requestSchema"
#define RESOURCE_DISCOVERY_REQUEST_BY_PROPERTIES "requestProperties"
#define RESOURCE_DISCOVERY_REQUEST_COMPONENTS "requestComponents"




#endif //UBIFORM_SYSTEMENUMS_H
