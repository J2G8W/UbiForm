#include "gtest/gtest.h"

#include "Component.h"
class ComponentExample : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    ComponentExample(){
        component = new Component();
        if (pFile == NULL){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        component->specifyManifest(pFile);
    }

    ~ComponentExample(){
        delete component;
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    Component* component;
};


TEST_F(ComponentExample, GetEndpoints){
    component->createNewPairEndpoint("v1","TestEndpoint");

    ASSERT_NO_THROW(component->getReceiverEndpoint("TestEndpoint"));
    ASSERT_NO_THROW(component->getSenderEndpoint("TestEndpoint"));

    ASSERT_ANY_THROW(component->getSenderEndpoint("NotAnEndpoint"));
    ASSERT_ANY_THROW(component->getReceiverEndpoint("NotAnEndpoint"));
}

TEST_F(ComponentExample, WrongEndpointType){
    ASSERT_ANY_THROW(component->createNewPairEndpoint("NotAType","TestEndpoint"));
}
