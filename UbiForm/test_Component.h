#include "gtest/gtest.h"

#include "Component.h"
class SimpleComponent : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleComponent(){
        component = new Component();
        if (pFile == NULL){
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        component->specifyManifest(pFile);
    }

    ~SimpleComponent(){
        delete component;
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    Component* component;
};


TEST_F(SimpleComponent, CreateMultipleEndpoints){
    component->createNewPairEndpoint("pairExample", "PairEndpoint1");
    component->createNewPairEndpoint("pairExample", "PairEndpoint2");

    ASSERT_NO_THROW(component->getReceiverEndpointById("PairEndpoint1"));
    ASSERT_NO_THROW(component->getSenderEndpointById("PairEndpoint2"));
}

TEST_F(SimpleComponent, GetEndpoints){
    component->createNewPairEndpoint("pairExample","TestEndpoint");

    ASSERT_NO_THROW(component->getReceiverEndpointById("TestEndpoint"));
    ASSERT_NO_THROW(component->getSenderEndpointById("TestEndpoint"));

    ASSERT_ANY_THROW(component->getSenderEndpointById("NotAnEndpoint"));
    ASSERT_ANY_THROW(component->getReceiverEndpointById("NotAnEndpoint"));
}

TEST_F(SimpleComponent, WrongEndpointType){
    ASSERT_ANY_THROW(component->createNewPairEndpoint("NotAType","TestEndpoint"));
}

class PairBasedComponent : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    PairBasedComponent(){
        receiverComponent = new Component();
        senderComponent = new Component();
        if (pFile == NULL){
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        receiverComponent->specifyManifest(pFile);
        fseek(pFile,0, SEEK_SET);
        senderComponent->specifyManifest(pFile);

    }

    ~PairBasedComponent(){
        delete receiverComponent;
        delete senderComponent;
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    Component* receiverComponent;
    Component* senderComponent;
};

TEST_F(PairBasedComponent, FindEachOther){
    const char *address = "ipc:///tmp/test.ipc";
    // We use IPC to test our component
    senderComponent->startBackgroundListen(address);
    ASSERT_NO_THROW(receiverComponent->requestAndCreateConnection("pairExample",address, "pairExample"));

    sleep(1);
    // No throw means that there is in fact a pair connection being created in our component
    ASSERT_NO_THROW(receiverComponent->getReceiverEndpointsByType("pairExample")->at(0));
    ASSERT_NO_THROW(senderComponent->getSenderEndpointsByType("pairExample")->at(0));

}