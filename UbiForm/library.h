#ifndef UBIFORM_LIBRARY_H
#define UBIFORM_LIBRARY_H

#include <string>


#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "nng/nng.h"

class ComponentManifest{
private:
    rapidjson::Document JSON_document;


public:
    // Accept JSON input as string
    explicit ComponentManifest(const char* jsonString){
        rapidjson::StringStream stream(jsonString);
        JSON_document.ParseStream(stream);
    };
    // Accept JSON input as a FILE pointer
    explicit ComponentManifest(FILE* jsonFP){
        char readBuffer[65536];
        rapidjson::FileReadStream inputStream(jsonFP, readBuffer, sizeof(readBuffer));
        JSON_document.ParseStream(inputStream);
    };

    std::string getName();

    const char* stringify(){
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        JSON_document.Accept(writer);
        return buffer.GetString();
    }

};

class Component{
private:
    ComponentManifest manifest;
    nng_socket socket;

public:
    void specifyManifest(FILE* jsonFP){manifest  = ComponentManifest(jsonFP);}
    void specifyManifest(const char *jsonString){manifest = ComponentManifest(jsonString);}

    void createPairConnectionOutgoing(const char* url);
    void createPairConnectionIncoming(const char* url);

    void sendManifestOnSocket();
    void receiveManifestOnSocket();



};

#endif //UBIFORM_LIBRARY_H
