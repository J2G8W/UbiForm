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
    bool reached = false;
};
void simplePairFunc(Endpoint* e, void* extraData){
    auto* ex = static_cast<ExtraData*>(extraData);
    ASSERT_NO_THROW(ex->component->castToPair(e));
    ASSERT_THROW(ex->component->castToSubscriber(e),AccessError);
    ex->reached = true;
}
TEST_F(EndpointCreation, RegisterFunction){
    ASSERT_NO_THROW(component.registerStartupFunction("NOT VALID", nullptr, nullptr));
    auto* e = new ExtraData;
    e->component = &component;
    component.registerStartupFunction("pairExample",simplePairFunc,e);
    component.createEndpointAndListen("pairExample");
    nng_msleep(100);
    ASSERT_TRUE(e->reached);
    delete e;
}