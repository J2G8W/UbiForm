#include "gtest/gtest.h"

#include "Component.h"
class SimpleComponent : public testing::Test{
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleComponent(){
        component = new Component();
        if (pFile == NULL){
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        component->specifyManifest(pFile);
    }

    ~SimpleComponent(){
        delete component;
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    Component* component;
};


TEST_F(SimpleComponent, CreateMultipleEndpoints){
    component->createNewPublisherEndpoint("v1", "PubEndpoint");
    component->createNewSubscriberEndpoint("v1", "SubEndpoint");

    ASSERT_NO_THROW(component->getReceiverEndpointById("SubEndpoint"));
    ASSERT_NO_THROW(component->getSenderEndpointById("PubEndpoint"));
}

TEST_F(SimpleComponent, GetEndpoints){
    component->createNewPairEndpoint("v1","TestEndpoint");

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
            std::cerr << "Error finding requisite file (JsonFiles/PairManifest1.json)";
        }
        receiverComponent->specifyManifest(pFile);
        fseek(pFile,0, SEEK_SET);
        senderComponent->specifyManifest(pFile);

    }

    ~PairBasedComponent(){
        delete receiverComponent;
        delete senderComponent;
    }

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    Component* receiverComponent;
    Component* senderComponent;
};

TEST_F(PairBasedComponent, FindEachOther){
    const char *address = "ipc:///tmp/test.ipc";
    // We use IPC to test our component
    senderComponent->startBackgroundListen(address);
    ASSERT_NO_THROW(receiverComponent->requestPairConnection(address, "v1"));

    sleep(1);
    ASSERT_NO_THROW(receiverComponent->getReceiverEndpointsByType("v1")->at(0));
    ASSERT_NO_THROW(senderComponent->getSenderEndpointsByType("v1")->at(0));

}