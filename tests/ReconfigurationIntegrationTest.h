TEST(ReconfigurationIntegrationTest, IntegrationTest1){
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");


    receiverComponent.specifyManifest(R"({"name":"TEST1","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"TEST2","schemas":{}})");

    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    auto newEs = std::make_shared<EndpointSchema>();
    newEs->addProperty("msg", ValueType::String);
    newEs->addRequired("msg");
    senderComponent.getBackgroundRequester().requestChangeEndpoint(receiverComponent.getBackgroundListenAddress(),
                                                                   SocketType::Subscriber,
                                                                   "generatedSubscriber",
                                                                   newEs.get(),
                                                                   nullptr);
    senderComponent.getComponentManifest().addEndpoint(SocketType::Publisher,"generatedPublisher",
                                                        nullptr, newEs);
    nng_msleep(300);

    ASSERT_NO_THROW(senderComponent.getComponentManifest().getSenderSchema("generatedPublisher"));
    ASSERT_NO_THROW(receiverComponent.getComponentManifest().getReceiverSchema("generatedSubscriber"));

    senderComponent.getBackgroundRequester().tellToRequestAndCreateConnection(receiverComponent.getBackgroundListenAddress(), "generatedSubscriber",
                                                                              "generatedPublisher", senderComponent.getBackgroundListenAddress());

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


TEST(ReconfigurationIntegrationTest, IntegrationTest2){
    Component RDH("ipc:///tmp/RDH");
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");

    RDH.specifyManifest(R"({"name":"HUB","schemas":{}})");
    receiverComponent.specifyManifest(R"({"name":"RECEIVER","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"SENDER","schemas":{}})");

    RDH.startResourceDiscoveryHub(7999);
    std::string rdhLocation = "ipc:///tmp/RDH:7999";

    RDH.startBackgroundListen();
    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    RDH.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhLocation);
    receiverComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhLocation);
    senderComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhLocation);


    ASSERT_EQ(RDH.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(),1);
    ASSERT_EQ(receiverComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(),1);
    ASSERT_EQ(senderComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(),1);

    std::string receiverComponentId = receiverComponent.getResourceDiscoveryConnectionEndpoint().getId(
            receiverComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().at(0));

    auto receiverRep = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhLocation, receiverComponentId);

    ASSERT_NO_THROW(receiverRep->getName());
    ASSERT_EQ(receiverRep->getName(), "RECEIVER");

    auto newEndpointSchema = std::make_shared<EndpointSchema>();
    newEndpointSchema->addProperty("value", ValueType::Number);
    newEndpointSchema->addRequired("value");
    receiverComponent.getComponentManifest().addEndpoint(SocketType::Subscriber, "genSubscriber",
                                                          newEndpointSchema, nullptr);
    receiverComponent.getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();

    receiverRep = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhLocation, receiverComponentId);

    ASSERT_NO_THROW(receiverRep->getReceiverSchema("genSubscriber"));
    ASSERT_EQ(newEndpointSchema->stringify(), receiverRep->getReceiverSchema("genSubscriber")->stringify());

    receiverComponent.getBackgroundRequester().requestChangeEndpoint(senderComponent.getBackgroundListenAddress(),
                                                                     SocketType::Publisher,
                                                                     "genPublisher",
                                                                     nullptr, newEndpointSchema.get());
    // Takes time for the background senderComponent to register with RDH
    nng_msleep(300);

    auto locations = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentsBySchema("genSubscriber");
    ASSERT_EQ(locations.size(),1);
    ASSERT_EQ(locations.at(0)->getString("url"), senderComponent.getBackgroundListenAddress());

    // Make two subscribers (but only one publisher)
    receiverComponent.getBackgroundRequester().requestAndCreateConnection(locations.at(0)->getString("url"),
                                                                          "genSubscriber", "genPublisher");
    receiverComponent.getBackgroundRequester().requestAndCreateConnection(locations.at(0)->getString("url"),
                                                                          "genSubscriber", "genPublisher");


    auto subscriberEndpoints = receiverComponent.getReceiverEndpointsByType("genSubscriber");
    ASSERT_EQ(subscriberEndpoints->size(),2);

    auto publisherEndpoints = senderComponent.getSenderEndpointsByType("genPublisher");
    ASSERT_EQ(publisherEndpoints->size(), 1);

    SocketMessage sm;
    sm.addMember("value", 42);
    publisherEndpoints->at(0)->asyncSendMessage(sm);

    auto receiverMsg = subscriberEndpoints->at(0)->receiveMessage();
    ASSERT_EQ(receiverMsg->getInteger("value"), 42);
}