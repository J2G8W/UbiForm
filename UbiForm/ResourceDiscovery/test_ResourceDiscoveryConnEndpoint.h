#include "ResourceDiscoveryConnEndpoint.h"

class test_RDC : public testing::Test{
protected:
    test_RDC() : component(), rdc(&component, component.getSystemSchemas()){
        FILE* pFile = fopen("TestManifests/Component1.json", "r");
        if (pFile == nullptr){
            std::cerr << "Error finding requisite file ("<<"TestManifests/Component1.json" <<")" << std::endl;
        }
        component.specifyManifest(pFile);
        fclose(pFile);
    }


    Component component;
    ResourceDiscoveryConnEndpoint rdc;
};

TEST_F(test_RDC, GenerateRegisterRequest){
    SocketMessage *request = nullptr;
    ASSERT_NO_THROW(request =rdc.generateRegisterRequest());
    delete request;
}