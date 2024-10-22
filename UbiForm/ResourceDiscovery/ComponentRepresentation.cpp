#include "../../include/UbiForm/ResourceDiscovery/ComponentRepresentation.h"

bool ComponentRepresentation::isEqual(const std::string &endpointId, bool recv, EndpointMessage &sm) {
    const auto &schemas = JSON_document["schemas"].GetObject();
    if (schemas.HasMember(endpointId) && schemas[endpointId].IsObject()) {
        if (recv && schemas[endpointId].GetObject().HasMember("receive") &&
            schemas[endpointId].GetObject()["receive"].IsObject()) {
            return compareSchemaObjects(schemas[endpointId].GetObject()["receive"], sm.JSON_document);
        }
        if (!recv &&
            (schemas[endpointId].GetObject().HasMember("send") && schemas[endpointId].GetObject()["send"].IsObject())) {
            return compareSchemaObjects(schemas[endpointId].GetObject()["send"], sm.JSON_document);
        }
    }
    return false;
}

std::vector<std::string> ComponentRepresentation::findEquals(bool recv, EndpointMessage &sm) {
    std::vector<std::string> returnIds;

    for (auto &v : JSON_document["schemas"].GetObject()) {
        if (this->isEqual(v.name.GetString(), recv, sm)) {
            returnIds.emplace_back(v.name.GetString());
        }
    }
    return returnIds;
}

void ComponentRepresentation::fillSelf() {
    if (JSON_document.HasMember("urls") && JSON_document["urls"].IsArray()) {
        for (const auto &u: JSON_document["urls"].GetArray()) {
            if (!u.IsString()) { throw ValidationError("Urls not of type string"); }
            urls.emplace_back(u.GetString());
        }
    } else {
        throw ValidationError("URLs of ComponentRepresentation not valid");
    }

    if (JSON_document.HasMember("port") && JSON_document["port"].IsInt()) {
        port = JSON_document["port"].GetInt();
    } else {
        throw ValidationError("No valid port for ComponentRepresentation");
    }

}
