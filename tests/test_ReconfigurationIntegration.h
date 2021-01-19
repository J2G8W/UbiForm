TEST(ReconfigurationIntegrationTest, IntegrationTest1) {
    // Startup
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");


    receiverComponent.specifyManifest(R"({"name":"TEST1","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"TEST2","schemas":{}})");

    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    auto newEs = std::make_shared<EndpointSchema>();
    newEs->addProperty("msg", ValueType::String);
    newEs->addRequired("msg");

    std::string receiverCompAddress = "ipc:///tmp/comp1:" + std::to_string(receiverComponent.getBackgroundPort());
    std::string senderCompAddress = "ipc:///tmp/comp2:" + std::to_string(senderComponent.getBackgroundPort());

    // Test Request change endpoint
    senderComponent.getBackgroundRequester().requestChangeEndpoint(receiverCompAddress,
                                                                   SocketType::Subscriber,
                                                                   "generatedSubscriber",
                                                                   newEs.get(),
                                                                   nullptr);
    ASSERT_NO_THROW(receiverComponent.getComponentManifest().getReceiverSchema("generatedSubscriber"));

    // Test local change endpoint
    senderComponent.getComponentManifest().addEndpoint(SocketType::Publisher, "generatedPublisher",
                                                       nullptr, newEs);
    ASSERT_NO_THROW(senderComponent.getComponentManifest().getSenderSchema("generatedPublisher"));


    // Test tellToRequestAndCreateConnection
    senderComponent.getBackgroundRequester().tellToRequestAndCreateConnection(receiverCompAddress,
                                                                              "generatedSubscriber",
                                                                              "generatedPublisher", "ipc:///tmp/comp2",
                                                                              senderComponent.getBackgroundPort());

    SocketMessage original;
    original.addMember("msg", "HELLO WORLD");

    nng_msleep(300);

    auto senderEndpoints = senderComponent.getSenderEndpointsByType("generatedPublisher");
    ASSERT_GT(senderEndpoints->size(), 0);
    auto receiverEndpoints = receiverComponent.getReceiverEndpointsByType("generatedSubscriber");
    ASSERT_GT(receiverEndpoints->size(), 0);

    // Test simple Async Message
    senderEndpoints->at(0)->asyncSendMessage(original);
    auto receiveOriginal = receiverEndpoints->at(0)->receiveMessage();

    ASSERT_EQ(original.getString("msg"), receiveOriginal->getString("msg"));

    // Test RequestCloseSocketOfType
    auto subEndpoint = receiverEndpoints->at(0);
    senderComponent.getBackgroundRequester().requestCloseSocketOfType(receiverCompAddress, "generatedSubscriber");
    ASSERT_EQ(receiverEndpoints->size(), 0);
    ASSERT_THROW(subEndpoint->receiveMessage(), SocketOpenError);

    // Test local close socket
    std::string senderEndpointID = senderEndpoints->at(0)->getSenderEndpointID();
    senderComponent.closeSocketOfId(senderEndpointID);
    ASSERT_EQ(senderEndpoints->size(), 0);
    ASSERT_THROW(senderComponent.getSenderEndpointById(senderEndpointID), std::out_of_range);
}


TEST(ReconfigurationIntegrationTest, IntegrationTest2) {
    //Startup
    Component RDH("ipc:///tmp/RDH");
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");

    RDH.specifyManifest(R"({"name":"HUB","schemas":{}})");
    receiverComponent.specifyManifest(R"({"name":"RECEIVER","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"SENDER","schemas":{}})");

    RDH.startBackgroundListen();
    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    std::string receiverFullAddress = receiverComponent.getSelfAddress() + ":" + std::to_string(receiverComponent.getBackgroundPort());
    std::string senderFullAddress = senderComponent.getSelfAddress() + ":" + std::to_string(senderComponent.getBackgroundPort());

    // Test creation and self add of RDH
    RDH.startResourceDiscoveryHub(7999);
    std::string rdhLocation = "ipc:///tmp/RDH:7999";
    ASSERT_EQ(RDH.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(), 1);

    // Test local adding to RDH
    receiverComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhLocation);
    senderComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhLocation);

    ASSERT_EQ(receiverComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(), 1);
    ASSERT_EQ(senderComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(), 1);

    // Test getComponentByID in Resource Discovery
    std::string receiverComponentId = receiverComponent.getResourceDiscoveryConnectionEndpoint().getId(
            receiverComponent.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().at(0));

    auto receiverRep = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhLocation,
                                                                                                   receiverComponentId);
    ASSERT_NO_THROW(receiverRep->getName());
    ASSERT_EQ(receiverRep->getName(), "RECEIVER");

    // Test updateManifestWithHubs
    auto newEndpointSchema = std::make_shared<EndpointSchema>();
    newEndpointSchema->addProperty("value", ValueType::Number);
    newEndpointSchema->addRequired("value");
    receiverComponent.getComponentManifest().addEndpoint(SocketType::Subscriber, "genSubscriber",
                                                         newEndpointSchema, nullptr);
    receiverComponent.getResourceDiscoveryConnectionEndpoint().updateManifestWithHubs();

    receiverRep = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhLocation,
                                                                                              receiverComponentId);

    ASSERT_NO_THROW(receiverRep->getReceiverSchema("genSubscriber"));
    ASSERT_EQ(newEndpointSchema->stringify(), receiverRep->getReceiverSchema("genSubscriber")->stringify());

    // Test that changing endpoint does remote updateManifestWithHubs
    receiverComponent.getBackgroundRequester().requestChangeEndpoint(senderFullAddress,
                                                                     SocketType::Publisher,
                                                                     "genPublisher",
                                                                     nullptr, newEndpointSchema.get());
    // Takes time for the background senderComponent to register with RDH
    nng_msleep(300);

    std::map<std::string, std::string> empty;
    auto locations = receiverComponent.getResourceDiscoveryConnectionEndpoint().getComponentsBySchema("genSubscriber",
                                                                                                      empty);
    ASSERT_EQ(locations.size(), 1);
    ASSERT_EQ(locations.at(0)->getInteger("port"), senderComponent.getBackgroundPort());
    auto urls = locations.at(0)->getArray<std::string>("urls");
    ASSERT_GT(urls.size(), 0);

    // Test requestRemoteThenDial
    receiverComponent.getBackgroundRequester().requestRemoteListenThenDial(urls.at(0),
                                                                           locations.at(0)->getInteger("port"),
                                                                           "genSubscriber", "genPublisher");
    receiverComponent.getBackgroundRequester().requestRemoteListenThenDial(urls.at(0),
                                                                           locations.at(0)->getInteger("port"),
                                                                           "genSubscriber", "genPublisher");


    auto subscriberEndpoints = receiverComponent.getReceiverEndpointsByType("genSubscriber");
    ASSERT_EQ(subscriberEndpoints->size(), 2);

    auto publisherEndpoints = senderComponent.getSenderEndpointsByType("genPublisher");
    ASSERT_EQ(publisherEndpoints->size(), 1);

    // Test simple message send
    SocketMessage sm;
    sm.addMember("value", 42);
    publisherEndpoints->at(0)->asyncSendMessage(sm);

    auto receiverMsg = subscriberEndpoints->at(0)->receiveMessage();
    ASSERT_EQ(receiverMsg->getInteger("value"), 42);

    // Test requestEndpointInfo
    auto reply = RDH.getBackgroundRequester().requestEndpointInfo(senderFullAddress);
    ASSERT_EQ(reply.size(),1);
    ASSERT_EQ(reply.at(0)->getString("endpointType"),"genPublisher");

    reply = RDH.getBackgroundRequester().requestEndpointInfo(receiverFullAddress);
    ASSERT_EQ(reply.size(),2);

}


TEST(ReconfigurationIntegrationTest, IntegrationTest3) {
    // Setupd
    Component receiverComponent("ipc:///tmp/comp1");
    Component senderComponent("ipc:///tmp/comp2");

    receiverComponent.specifyManifest(R"({"name":"RECEIVER","schemas":{}})");
    senderComponent.specifyManifest(R"({"name":"SENDER","schemas":{}})");

    receiverComponent.startBackgroundListen();
    senderComponent.startBackgroundListen();

    std::string receiverComponentFullAddress =
            receiverComponent.getSelfAddress() + ":" + std::to_string(receiverComponent.getBackgroundPort());

    // Test requestCreateRDH
    int rdhPort = senderComponent.getBackgroundRequester().requestCreateRDH(receiverComponentFullAddress);
    std::string rdhUrl = "ipc:///tmp/comp1:" + std::to_string(rdhPort);

    ASSERT_NO_THROW(senderComponent.getResourceDiscoveryConnectionEndpoint().registerWithHub(rdhUrl));
    senderComponent.getBackgroundRequester().requestAddRDH(receiverComponentFullAddress, rdhUrl);

    // Test getComponentById in Resource Discovery
    std::string receiverID = receiverComponent.getResourceDiscoveryConnectionEndpoint().getId(rdhUrl);

    auto cr = senderComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhUrl, receiverID);
    ASSERT_EQ(cr->getPort(), receiverComponent.getBackgroundPort());
    ASSERT_EQ(cr->getName(), receiverComponent.getComponentManifest().getName());


    // Test remote updateComponentManifest
    ComponentManifest cm(senderComponent.getSystemSchemas());
    std::string newName = "NEW RECEIVER";
    cm.setName(newName);
    senderComponent.getBackgroundRequester().requestUpdateComponentManifest(receiverComponentFullAddress, cm);

    ASSERT_EQ(receiverComponent.getComponentManifest().getName(), newName);

    // Tests updateManifestsWithHubs works
    cr = senderComponent.getResourceDiscoveryConnectionEndpoint().getComponentById(rdhUrl, receiverID);
    ASSERT_EQ(cr->getPort(), receiverComponent.getBackgroundPort());
    ASSERT_EQ(cr->getName(), newName);
}

TEST(ReconfigurationIntegrationTest, IntegrationTest4) {
    // Setup
    Component RDH1("ipc:///tmp/RDH1");
    Component RDH2("ipc:///tmp/RDH2");
    Component RDH3("ipc:///tmp/RDH3");
    Component baby("ipc:///tmp/baby");


    int port1 = RDH1.startResourceDiscoveryHub();
    int port2 = RDH2.startResourceDiscoveryHub();
    int port3 = RDH3.startResourceDiscoveryHub();

    std::string loc1 = "ipc:///tmp/RDH1:" + std::to_string(port1);
    std::string loc2 = "ipc:///tmp/RDH2:" + std::to_string(port2);
    std::string loc3 = "ipc:///tmp/RDH3:" + std::to_string(port3);

    RDH1.startBackgroundListen();
    RDH2.startBackgroundListen();
    RDH3.startBackgroundListen();
    baby.startBackgroundListen();

    std::string back1 = "ipc:///tmp/RDH1:" + std::to_string(RDH1.getBackgroundPort());

    // Test simple register with hub
    RDH1.getResourceDiscoveryConnectionEndpoint().registerWithHub(loc2);
    RDH1.getResourceDiscoveryConnectionEndpoint().registerWithHub(loc3);

    ASSERT_EQ(RDH1.getResourceDiscoveryConnectionEndpoint().getResourceDiscoveryHubs().size(), 3);

    // Test requestLocationsOfRDH
    std::vector<std::string> rdhLocations = baby.getBackgroundRequester().requestLocationsOfRDH(back1);
    ASSERT_EQ(rdhLocations.size(), 3);
    bool testers[3] = {false, false, false};
    for (const std::string &loc : rdhLocations) {
        if (loc == loc1) { testers[0] = true; }
        if (loc == loc2) { testers[1] = true; }
        if (loc == loc3) { testers[2] = true; }
    }
    for (bool &tester : testers) { ASSERT_TRUE(tester); }

    // Test deRegisterFromAllHubs
    RDH1.getResourceDiscoveryConnectionEndpoint().deRegisterFromAllHubs();
    rdhLocations = baby.getBackgroundRequester().requestLocationsOfRDH(back1);
    ASSERT_EQ(rdhLocations.size(), 0);

    auto compIds = baby.getResourceDiscoveryConnectionEndpoint().getComponentIdsFromHub(loc2);
    // loc2 Should only have itself register with it
    ASSERT_EQ(compIds.size(), 1);


    // Test remote close RDH
    baby.getBackgroundRequester().requestCloseRDH(
            RDH1.getSelfAddress() + ":" + std::to_string(RDH1.getBackgroundPort()));
    ASSERT_THROW(RDH1.getResourceDiscoveryHubPort(), std::logic_error);
    // Test restart of RDH
    RDH1.startResourceDiscoveryHub();
    ASSERT_NO_THROW(RDH1.getResourceDiscoveryHubPort());
}


TEST(ReconfigurationIntegrationTest, IntegrationTest5) {
    // Startup
    Component RDH("ipc:///tmp/RDH");
    Component component1("ipc:///tmp/component1");
    Component component2("ipc:///tmp/component2");
    Component component3("ipc:///tmp/component3");

    RDH.getComponentManifest().setName("RDH");
    component1.getComponentManifest().setName("component1");
    component2.getComponentManifest().setName("component2");
    component3.getComponentManifest().setName("component3");

    RDH.getComponentManifest().setProperty("type", "RDH");
    component3.getComponentManifest().setProperty("type", "boring");
    component2.getComponentManifest().setProperty("type", "boring");

    RDH.startResourceDiscoveryHub();

    std::string RdhAddress = RDH.getSelfAddress() + ":" + std::to_string(RDH.getResourceDiscoveryHubPort());

    component1.getResourceDiscoveryConnectionEndpoint().registerWithHub(RdhAddress);
    component2.getResourceDiscoveryConnectionEndpoint().registerWithHub(RdhAddress);
    component3.getResourceDiscoveryConnectionEndpoint().registerWithHub(RdhAddress);

    // Test getComponentsByProperties
    std::map<std::string, std::string> nameMap;
    nameMap.insert(std::make_pair("name", "component2"));
    auto receivedNameMap = component1.getResourceDiscoveryConnectionEndpoint().getComponentsByProperties(nameMap);
    ASSERT_EQ(receivedNameMap.size(), 1);
    ASSERT_NO_THROW(receivedNameMap.at(component2.getResourceDiscoveryConnectionEndpoint().getId(RdhAddress)));

    std::map<std::string, std::string> typeMap;
    typeMap.insert(std::make_pair("type", "boring"));
    auto receiverTypeMap = component1.getResourceDiscoveryConnectionEndpoint().getComponentsByProperties(typeMap);
    ASSERT_EQ(receiverTypeMap.size(), 2);
    ASSERT_NO_THROW(receiverTypeMap.at(component2.getResourceDiscoveryConnectionEndpoint().getId(RdhAddress)));
    ASSERT_NO_THROW(receiverTypeMap.at(component3.getResourceDiscoveryConnectionEndpoint().getId(RdhAddress)));
}

TEST(ReconfigurationIntegrationTest, IntegrationTest6){
    // Setup
    Component component1("ipc:///tmp/component1");
    Component component2("ipc:///tmp/component2");

    std::shared_ptr<EndpointSchema> schema = std::make_shared<EndpointSchema>();
    schema->addProperty("TEST",ValueType::Number);

    component1.getComponentManifest().addEndpoint(SocketType::Subscriber,"SUB",schema, nullptr);
    component2.getComponentManifest().addEndpoint(SocketType::Publisher, "PUB", nullptr,schema);

    component1.startBackgroundListen();
    component2.startBackgroundListen();

    std::string component1Address = component1.getSelfAddress() + ":" + std::to_string(component1.getBackgroundPort());

    // Test localListenThenRequestRemoteDial
    ASSERT_NO_THROW(component2.getBackgroundRequester().localListenThenRequestRemoteDial(
                component1.getSelfAddress()+":"+std::to_string(component1.getBackgroundPort()),
                "PUB", "SUB"));

    ASSERT_EQ(component2.getSenderEndpointsByType("PUB")->size(),1);
    ASSERT_EQ(component1.getReceiverEndpointsByType("SUB")->size(),1);

    // Test requestCloseSocketOfId
    auto endpointInfo = component2.getBackgroundRequester().requestEndpointInfo(component1Address);
    ASSERT_GT(endpointInfo.size(),0);

    component2.getBackgroundRequester().requestCloseSocketOfId(component1Address,endpointInfo.at(0)->getString("id"));
    ASSERT_EQ(component1.getReceiverEndpointsByType("SUB")->size(),0);

    // Test changing Manifest with hanging pointer to socket
    auto pubEndpoint = component2.getSenderEndpointsByType("PUB")->at(0);
    component2.specifyManifest(R"({"name":"TEST1","schemas":{}})");
    pubEndpoint->listenForConnection("ipc:///tmp/component2",2000);
}