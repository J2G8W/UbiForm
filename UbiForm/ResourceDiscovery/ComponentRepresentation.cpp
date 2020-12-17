#include "ComponentRepresentation.h"

bool ComponentRepresentation::isEqual(std::string endpointId, bool recv, SocketMessage &sm) {
    const auto & schemas = JSON_document["schemas"].GetObject();
    if (schemas.HasMember(endpointId) && schemas[endpointId].IsObject()){
        if (recv && schemas[endpointId].GetObject().HasMember("receive") && schemas[endpointId].GetObject()["receive"].IsObject()){
            return compareSchemaObjects(schemas[endpointId].GetObject()["receive"], sm.JSON_document);
        }
        if (!recv && (schemas[endpointId].GetObject().HasMember("send") && schemas[endpointId].GetObject()["send"].IsObject())){
            return compareSchemaObjects(schemas[endpointId].GetObject()["send"], sm.JSON_document);
        }
    }
    return false;
}

std::vector<std::string> ComponentRepresentation::findEquals(bool recv, SocketMessage &sm) {
    std::vector<std::string> returnIds;

    for (auto &v : JSON_document["schemas"].GetObject()){
        if (this->isEqual(v.name.GetString(), recv, sm)){
            returnIds.emplace_back(v.name.GetString());
        }
    }
    return returnIds;
}

SocketMessage *ComponentRepresentation::getSchema(std::string endpointId, bool recv) {
    if (JSON_document["schemas"].GetObject().HasMember(endpointId)){
        SocketMessage *schema;
        if (recv) {
            schema = new SocketMessage(JSON_document["schemas"].GetObject()[endpointId].GetObject()["receive"]);
        }else{
            schema = new SocketMessage(JSON_document["schemas"].GetObject()[endpointId].GetObject()["send"]);
        }
        return schema;
    }
    return nullptr;
}
