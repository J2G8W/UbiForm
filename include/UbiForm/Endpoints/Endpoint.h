#ifndef UBIFORM_ENDPOINT_H
#define UBIFORM_ENDPOINT_H

#include "../Utilities/SystemEnums.h"


class Endpoint {
protected:
    EndpointState endpointState = EndpointState::Closed;
public:
    EndpointState getEndpointState(){return endpointState;}
};


#endif //UBIFORM_ENDPOINT_H
