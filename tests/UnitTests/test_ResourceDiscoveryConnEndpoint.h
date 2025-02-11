#include "../../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h"

class test_RDC : public testing::Test {
protected:
    test_RDC() : component("ipc:///tmp/RDH"), rdc(&component, component.getSystemSchemas()) {
        FILE *pFile = fopen("TestManifests/Component1.json", "r");
        if (pFile == nullptr) {
            std::cerr << "Error finding requisite file (" << "TestManifests/Component1.json" << ")" << std::endl;
        }
        component.specifyManifest(pFile);
        fclose(pFile);
    }


    Component component;
    ResourceDiscoveryConnEndpoint rdc;
};

TEST_F(test_RDC, GenerateRegisterRequest) {
    std::unique_ptr<EndpointMessage> request;
    ASSERT_NO_THROW(request = rdc.generateRegisterRequest());
    ASSERT_NO_THROW(component.getSystemSchemas().getSystemSchema(SystemSchemaName::additionRequest).validate(*request));
}

TEST_F(test_RDC, GenerateSchemaRequest){
    std::unique_ptr<EndpointMessage> request;
    std::map<std::string,std::string> props;
    props.insert(std::pair<std::string,std::string>("name","test1"));
    ASSERT_NO_THROW(request = rdc.generateFindBySchemaRequest("pairExample",props));
}