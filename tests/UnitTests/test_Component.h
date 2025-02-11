#include "gtest/gtest.h"

#include "../../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>

TEST(EmptyComponent, EmptyManifest) {
    Component component("ipc:///tmp/EMPTY");
    ASSERT_THROW(component.getReceiverEndpointById("some_type"), std::out_of_range);
}

TEST(EmptyComponent, DeleteComponent) {
    Component component("ipc:///tmp/BORING");
    component.specifyManifest("{\"name\":\"TEST1\",\"schemas\":{}}");
    component.startBackgroundListen();
    component.startResourceDiscoveryHub(7999);
    component.getResourceDiscoveryConnectionEndpoint();
    component.getBackgroundRequester();
}

class SimpleComponent : public testing::Test {
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    SimpleComponent() {
        component = new Component("ipc:///tmp/RDH");
        if (pFile == nullptr) {
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        component->specifyManifest(pFile);
    }

    ~SimpleComponent() override {
        delete component;
    }

    FILE *pFile = fopen("TestManifests/Component1.json", "r");
    Component *component;
};


TEST_F(SimpleComponent, CreateMultipleEndpoints) {
    component->createNewEndpoint("pairExample", "PairEndpoint1");
    component->createNewEndpoint("pairExample", "PairEndpoint2");

    ASSERT_NO_THROW(component->getReceiverEndpointById("PairEndpoint1"));
    ASSERT_NO_THROW(component->getSenderEndpointById("PairEndpoint2"));
}

TEST_F(SimpleComponent, GetEndpoints) {
    component->createNewEndpoint("pairExample", "TestEndpoint");

    ASSERT_NO_THROW(component->getReceiverEndpointById("TestEndpoint"));
    ASSERT_NO_THROW(component->getSenderEndpointById("TestEndpoint"));

    ASSERT_ANY_THROW(component->getSenderEndpointById("NotAnEndpoint"));
    ASSERT_ANY_THROW(component->getReceiverEndpointById("NotAnEndpoint"));
}

TEST_F(SimpleComponent, WrongEndpointType) {
    ASSERT_THROW(component->createNewEndpoint("NotAType", "TestEndpoint"), AccessError);
}


class PairBasedComponent : public testing::Test {
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    PairBasedComponent() {
        receiverComponent = new Component("ipc:///tmp/comp1");
        senderComponent = new Component("ipc:///tmp/comp2");
        if (pFile == nullptr) {
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        receiverComponent->specifyManifest(pFile);
        fseek(pFile, 0, SEEK_SET);
        senderComponent->specifyManifest(pFile);
        fclose(pFile);

    }

    ~PairBasedComponent() override {
        delete receiverComponent;
        delete senderComponent;
    }

    FILE *pFile = fopen("TestManifests/Component1.json", "r");
    Component *receiverComponent;
    Component *senderComponent;
};

TEST_F(PairBasedComponent, FindEachOther) {
    // We use IPC to test our component - concept of ports is less clear, but it works
    senderComponent->startBackgroundListen(8000);
    ASSERT_NO_THROW(receiverComponent->getBackgroundRequester().requestRemoteListenThenDial("ipc:///tmp/comp2", 8000,
                                                                                            "pairExample",
                                                                                            "pairExample"));

    nng_msleep(1000);
    // No throw means that there is in fact a pair connection being created in our component
    ASSERT_EQ(receiverComponent->getEndpointsByType("pairExample")->size(),1);
    ASSERT_EQ(senderComponent->getEndpointsByType("pairExample")->size(),1);
}
