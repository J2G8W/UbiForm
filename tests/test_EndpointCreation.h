class EndpointCreation : public testing::Test {
protected:
    // Note we aren't REALLY testing the inputting of the manifest as this is done automatically
    EndpointCreation() : component("ipc:///tmp/RDH") {
        if (pFile == nullptr) {
            std::cerr << "Error finding requisite file - " << "TestManifests/Component1.json" << std::endl;
        }
        component.specifyManifest(pFile);
    }

    ~EndpointCreation() override {
    }

    FILE *pFile = fopen("TestManifests/Component1.json", "r");
    Component component;
};


struct ExtraData{
    Component* component;
    int reached = 0;
};
void simplePairFunc(Endpoint* e, void* extraData){
    auto* ex = static_cast<ExtraData*>(extraData);
    ASSERT_NO_THROW(ex->component->castToPair(e));
    ex->reached++;
}
TEST_F(EndpointCreation, RegisterFunction){
    ASSERT_NO_THROW(component.registerStartupFunction("NOT VALID", nullptr, nullptr));
    auto* e = new ExtraData;
    e->component = &component;
    component.registerStartupFunction("pairExample",simplePairFunc,e);
    component.createEndpointAndListen("pairExample");
    nng_msleep(100);
    ASSERT_EQ(e->reached,1);
    delete e;
}

TEST_F(EndpointCreation, MultipleEndpoints){
    auto* e = new ExtraData;
    e->component = &component;
    component.registerStartupFunction("pairExample",simplePairFunc,e);
    component.createEndpointAndListen("pairExample");
    component.createEndpointAndListen("pairExample");
    component.createEndpointAndListen("pairExample");
    component.createEndpointAndListen("pairExample");
    nng_msleep(100);
    ASSERT_EQ(e->reached,4);
    delete e;
}

TEST_F(EndpointCreation, ReListen) {
    auto *e = new ExtraData;
    e->component = &component;
    component.registerStartupFunction("pairExample", simplePairFunc, e);
    component.createEndpointAndListen("pairExample");
    auto endpoints = component.getEndpointsByType("pairExample");
    ASSERT_EQ(endpoints->size(),1);
    endpoints->at(0)->closeEndpoint();
    component.castToPair(endpoints->at(0))->openEndpoint();
    component.castToPair(endpoints->at(0))->listenForConnection(component.getSelfAddress().c_str(),10000);
    nng_msleep(100);
    ASSERT_EQ(e->reached, 2);
    delete e;
}