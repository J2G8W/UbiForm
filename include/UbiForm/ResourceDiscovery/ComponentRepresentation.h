#ifndef UBIFORM_COMPONENTREPRESENTATION_H
#define UBIFORM_COMPONENTREPRESENTATION_H

#include "../ComponentManifest.h"

/**
 * This extends ComponentManifest and is the way we store Components in our ResourceDiscoveryHub. It is a normal manifest
 * but also has an ARRAY of urls which the component can be reached on and a port which the Component's background listener is on
 */
class ComponentRepresentation : public ComponentManifest {
private:
    std::string url;
    std::vector<std::string> urls;
    int port = -1;
public:
    explicit ComponentRepresentation(const char *JSON_input, SystemSchemas &ss) : ComponentManifest(JSON_input, ss) {
        fillSelf();
    }

    explicit ComponentRepresentation(EndpointMessage *sm, SystemSchemas &ss) : ComponentManifest(sm, ss) {
        fillSelf();
    }

    explicit ComponentRepresentation(FILE *jsonFP, SystemSchemas &ss) : ComponentManifest(jsonFP, ss) {
        fillSelf();
    }

    void fillSelf();


    std::vector<std::string> &getAllUrls() { return urls; }

    int getPort() { return port; }

    /**
     * Compare the equality of two schemas, one from our ComponentRepresentation and one from the EndpointMessage given. Will return false
     * if can't find the given endpoint in our ComponentRepresentation
     * @param endpointId - The ID of the endpoint we want to look at for our ComponentRepresentation
     * @param recv - Whether we want the receive or send schema of the given endpoint
     * @param sm - EndpointMessage object we are comparing to
     * @return Boolean whether the two things were equal (as defined by Julian's rules)
     */
    bool isEqual(const std::string &endpointId, bool recv, EndpointMessage &sm);

    /**
     * Find all the equal endpoints in the ComponentRepresentation to the given EndpointMessage
     * @param recv - Whether we are considering the receive or send schemas
     * @param sm - EndpointMessage we are comparing to
     * @return An array of the endpointTypes which are equal to the EndpointMessage given
     */
    std::vector<std::string> findEquals(bool recv, EndpointMessage &sm);


};


#endif //UBIFORM_COMPONENTREPRESENTATION_H
