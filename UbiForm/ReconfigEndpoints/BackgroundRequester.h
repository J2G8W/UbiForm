#ifndef UBIFORM_BACKGROUNDREQUESTER_H
#define UBIFORM_BACKGROUNDREQUESTER_H

#include "../SystemSchemas/SystemSchemas.h"

class Component;
class BackgroundRequester {
    Component * component;
    SystemSchemas& systemSchemas;
private:
    static std::string requestConnection(const std::string &address, const std::string &requestText);
public:
    BackgroundRequester(Component* c , SystemSchemas& ss):component(c), systemSchemas(ss){}

    void requestAndCreateConnection(const std::string &localEndpointType, const std::string &connectionComponentAddress,
                                    const std::string &remoteEndpointType);
};


#endif //UBIFORM_BACKGROUNDREQUESTER_H
