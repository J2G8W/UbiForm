class ReconfigurationIntegrationTest : public testing::Test{
protected:

    ReconfigurationIntegrationTest(): receiverComponent("ipc:///tmp/comp1"), senderComponent("ipc:///tmp/comp2"){
        receiverComponent.specifyManifest(R"({"name":"TEST1","schemas":{}})");
        senderComponent.specifyManifest(R"({"name":"TEST2","schemas":{}})");
    }

    ~ReconfigurationIntegrationTest() override = default;

    Component receiverComponent;
    Component senderComponent;
};

TEST_F(ReconfigurationIntegrationTest, IntegrationTest1){
    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    auto newEs = std::make_shared<EndpointSchema>();
    newEs->addProperty("msg", ValueType::String);
    newEs->addRequired("msg");
    senderComponent.getBackgroundRequester().requestAddEndpoint(receiverComponent.getBackgroundListenAddress(),"generatedSubscriber",
                                                                nullptr, newEs.get(),SocketType::Subscriber);
    senderComponent.getComponentManifest()->addEndpoint(SocketType::Publisher,"generatedPublisher",
                                                        nullptr, newEs);
    nng_msleep(300);
    std::cout << senderComponent.getComponentManifest()->stringify() << std::endl;
    std::cout << receiverComponent.getComponentManifest()->getReceiverSchema("generatedSubscriber")->stringify() << std::endl;
    senderComponent.requestAndCreateConnection(receiverComponent.getBackgroundListenAddress(),
                                               "generatedPublisher", "generatedSubscriber");

    SocketMessage original;
    original.addMember("msg","HELLO WORLD");

    nng_msleep(300);

    auto senderEndpoints = senderComponent.getSenderEndpointsByType("generatedPublisher");
    ASSERT_GT(senderEndpoints->size(),0);
    auto receiverEndpoints = receiverComponent.getReceiverEndpointsByType("generatedSubscriber");
    ASSERT_GT(receiverEndpoints->size(),0);

    senderEndpoints->at(0)->asyncSendMessage(original);
    auto receiveOriginal = receiverEndpoints->at(0)->receiveMessage();

    ASSERT_EQ(original.getString("msg"),receiveOriginal->getString("msg"));
}