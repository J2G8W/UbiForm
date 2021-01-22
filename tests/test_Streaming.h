#include "../UbiForm/Utilities/base64.h"


TEST(StreamingTests, SendMessage){
    // Use tcp as ipc doesn't guarantee ordering
    Component sendComponent("tcp://127.0.0.1");
    Component recvComponent("tcp://127.0.0.2");
    std::shared_ptr<EndpointSchema> es = std::make_shared<EndpointSchema>();
    es->addProperty("bytes",ValueType::String);

    sendComponent.getComponentManifest().addEndpoint(SocketType::Pair,"streamSender", es, es);
    recvComponent.getComponentManifest().addEndpoint(SocketType::Pair, "streamRecv",es, es);

    sendComponent.startBackgroundListen();
    recvComponent.startBackgroundListen();

    std::string recvComponentAddress = recvComponent.getSelfAddress() + ":" + std::to_string(recvComponent.getBackgroundPort());

    sendComponent.getBackgroundRequester().localListenThenRequestRemoteDial(recvComponentAddress,"streamSender","streamRecv");


    std::shared_ptr<DataReceiverEndpoint> uncastRecvEndpoint;
    std::shared_ptr<DataSenderEndpoint> uncastSendEndpoint;
    ASSERT_NO_THROW(uncastRecvEndpoint = recvComponent.getReceiverEndpointsByType("streamRecv")->at(0));
    ASSERT_NO_THROW(uncastSendEndpoint = sendComponent.getSenderEndpointsByType("streamSender")->at(0));

    std::shared_ptr<PairEndpoint> recvEndpoint;
    ASSERT_NO_THROW(recvEndpoint = recvComponent.castToPair(uncastRecvEndpoint));
    std::shared_ptr<PairEndpoint> sendEndpoint;
    ASSERT_NO_THROW(sendEndpoint = sendComponent.castToPair(uncastSendEndpoint));


    std::fstream fs;
    fs.open("TestFiles/test_image1.JPG", std::fstream::in|std::fstream::binary);
    if(!fs.good()){
        throw std::logic_error("Error with file");
    }
    fs.seekg(0, std::fstream::end);
    int fileSize = fs.tellg();
    fs.seekg(0, std::fstream::beg);

    sendEndpoint->sendStream(fs, 5001, false);
    std::fstream out;
    out.open("receive.jpg",std::fstream::out|std::fstream::binary);

    recvEndpoint->receiveStream(out);
    while(!recvEndpoint->getReceiverThreadEnded()) {
        nng_msleep(100);
    }

    ASSERT_EQ(fileSize,(int) out.tellp() - 1);
}

