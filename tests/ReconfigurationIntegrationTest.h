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

    auto subEndpoint = receiverEndpoints->at(0);
    senderComponent.getBackgroundRequester().requestCloseSocketOfType(receiverComponent.getBackgroundListenAddress(),"generatedSubscriber");
    ASSERT_EQ(receiverEndpoints->size(), 0);
    ASSERT_THROW(subEndpoint->receiveMessage(),SocketOpenError);
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


TEST(ReconfigurationIntegrationTest, IntegrationTest3){
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");

    receiverComponent.specifyManifest(R"({"name":"RECEIVER","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"SENDER","schemas":{}})");

    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    std::string url = senderComponent.getBackgroundRequester().requestCreateRDH(receiverComponent.getBackgroundListenAddress());

    ASSERT_NO_THROW(senderComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(url));
    senderComponent.getBackgroundRequester().requestAddRDH(receiverComponent.getBackgroundListenAddress(), url);

    std::string receiverID = receiverComponent.getResourceDiscoveryConnectionEndpoint().getId(url);

    auto cr = senderComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(url, receiverID);
    ASSERT_EQ(cr->getUrl(),receiverComponent.getBackgroundListenAddress());
    ASSERT_EQ(cr->getName(), receiverComponent.getComponentManifest().getName());


    ComponentManifest cm(senderComponent.getSystemSchemas());
    std::string newName = "NEW RECEIVER";
    cm.setName(newName);
    senderComponent.getBackgroundRequester().requestUpdateComponentManifest(receiverComponent.getBackgroundListenAddress(), cm);

    ASSERT_EQ(receiverComponent.getComponentManifest().getName(), newName);

    cr = senderComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(url, receiverID);
    ASSERT_EQ(cr->getUrl(),receiverComponent.getBackgroundListenAddress());
    ASSERT_EQ(cr->getName(), newName);
}

TEST(ReconfigurationIntegrationTest, IntegrationTest4){
    Component RDH1("ipc:///tmp/RDH1");
    Component RDH2("ipc:///tmp/RDH2");
    Component RDH3("ipc:///tmp/RDH3");
    Component baby("ipc:///tmp/baby");

    std::string loc1 = RDH1.startResourceDiscoveryHub();
    std::string loc2 = RDH2.startResourceDiscoveryHub();
    std::string loc3 = RDH3.startResourceDiscoveryHub();

    RDH1.startBackgroundListen();
    RDH2.startBackgroundListen();
    RDH3.startBackgroundListen();
    baby.startBackgroundListen();

    RDH1.getResourceDiscoveryConnectionEndpoint().registerWithHub(loc1);
    RDH1.getResourceDiscoveryConnectionEndpoint().registerWithHub(loc2);
    RDH1.getResourceDiscoveryConnectionEndpoint().registerWithHub(loc3);

    ASSERT_EQ(RDH1.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(), 3);
    std::vector<std::string> rdhLocations = baby.getBackgroundRequester().requestLocationsOfRDH(RDH1.getBackgroundListenAddress());

    ASSERT_EQ(rdhLocations.size(), 3);
    bool testers[3]={false,false,false};
    for(const std::string& loc : rdhLocations){
        if(loc == RDH1.getRDHLocation()){testers[0] = true;}
        if(loc == RDH2.getRDHLocation()){testers[1] = true;}
        if(loc == RDH3.getRDHLocation()){testers[2] = true;}
    }
    for(bool & tester : testers){ASSERT_TRUE(tester);}
}
