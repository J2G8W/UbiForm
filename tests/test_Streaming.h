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

    sendEndpoint->sendStream(fs, 2046, false);

    auto t1 = std::chrono::high_resolution_clock::now();
    std::string recvMessage;
    while(true) {
        auto message = recvEndpoint->receiveMessage();
        if(message->hasMember("end") && message->getBoolean("end")){
            break;
        }
        std::string r  = message->getString("bytes");
        recvMessage += r;
    }

    auto decode = base64_decode(recvMessage);
    ASSERT_EQ(fileSize,decode.size());
    std::fstream out;
    out.open("receive.jpg",std::fstream::out|std::fstream::binary);
    std::copy(decode.cbegin(),decode.cend(), std::ostream_iterator<unsigned char>(out));
    ASSERT_EQ(fileSize,out.tellp());

    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

    int kiloByteSize = fileSize/1024;

    std::cout << "Streaming of file of size: " << kiloByteSize <<"KB took " << duration << " milliseconds" << std::endl;
}

