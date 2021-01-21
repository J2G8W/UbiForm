#include "../UbiForm/Utilities/base64.h"

void doStuff(Component* c, std::iostream* fs){
    auto sendEndpoint = c->getSenderEndpointsByType("streamSender")->at(0);
    sendEndpoint->sendStream(*fs, 2001, false);
}


TEST(StreamingTests, SendMessage){
    Component sendComponent("ipc:///tmp/comp1");
    Component recvComponent("ipc:///tmp/comp2");
    std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
    es->addProperty("bytes",ValueType::String);

    sendComponent.getComponentManifest().addEndpoint(SocketType::Publisher,"streamSender", nullptr, es);
    recvComponent.getComponentManifest().addEndpoint(SocketType::Subscriber, "streamRecv",es, nullptr);

    sendComponent.startBackgroundListen();
    recvComponent.startBackgroundListen();

    std::string recvComponentAddress = recvComponent.getSelfAddress() + ":" + std::to_string(recvComponent.getBackgroundPort());

    sendComponent.getBackgroundRequester().localListenThenRequestRemoteDial(recvComponentAddress,"streamSender","streamRecv");


    auto recvEndpoint = recvComponent.getReceiverEndpointsByType("streamRecv")->at(0);
    auto sendEndpoint = sendComponent.getSenderEndpointsByType("streamSender")->at(0);


    std::fstream fs;
    fs.open("TestFiles/test_image1.JPG", std::fstream::in|std::fstream::binary);
    if(!fs.good()){
        throw std::logic_error("Error with file");
    }
    fs.seekg(0, std::fstream::end);
    int fileSize = fs.tellg();
    fs.seekg(0, std::fstream::beg);

    sendEndpoint->sendStream(fs, 10002, false);
    std::fstream out;
    out.open("receive.jpg",std::fstream::out|std::fstream::binary);

    recvEndpoint->receiveStream(out);
    while(!recvEndpoint->getReceiverThreadEnded()) {
        nng_msleep(100);
    }

    ASSERT_EQ(fileSize,(int) out.tellp() - 1);
}

