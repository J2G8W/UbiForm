#include "gtest/gtest.h"

#include "Component.h"
#include <nng/supplemental/util/platform.h>

TEST(EmptyComponent, NoManifest){
    Component component;
    ASSERT_THROW(component.getComponentManifest(), std::logic_error);
    ASSERT_THROW(component.getReceiverEndpointById("some_type"), std::out_of_range);
}

TEST(EmptyComponent, DeleteComponent){
    Component component;
    component.specifyManifest("{\"name\":\"TEST1\",\"schemas\":{}}");
    component.startBackgroundListen();
    component.startResourceDiscoveryHub(7999);
    component.getResourceDiscoveryConnectionEndpoint();
    component.getBackgroundRequester();
}

class SimpleComponent : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleComponent(){
        component = new Component();
        if (pFile == nullptr){
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        component->specifyManifest(pFile);
    }

    ~SimpleComponent() override{
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
        receiverComponent = new Component("ipc:///tmp/comp1");
        senderComponent = new Component("ipc:///tmp/comp2");
        if (pFile == nullptr){
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        receiverComponent->specifyManifest(pFile);
        fseek(pFile,0, SEEK_SET);
        senderComponent->specifyManifest(pFile);
        fclose(pFile);

    }

    ~PairBasedComponent() override{
        delete receiverComponent;
        delete senderComponent;
    }

    FILE* pFile = fopen("TestManifests/Component1.json", "r");
    Component* receiverComponent;
    Component* senderComponent;
};

TEST_F(PairBasedComponent, FindEachOther){
    // We use IPC to test our component - concept of ports is less clear, but it works
    senderComponent->startBackgroundListen(8000);
    ASSERT_NO_THROW(receiverComponent->requestAndCreateConnection(senderComponent->getBackgroundListenAddress(),
                                                                  "pairExample", "pairExample"));

    nng_msleep(1000);
    // No throw means that there is in fact a pair connection being created in our component
    ASSERT_NO_THROW(receiverComponent->getReceiverEndpointsByType("pairExample")->at(0));
    ASSERT_NO_THROW(senderComponent->getSenderEndpointsByType("pairExample")->at(0));
}
