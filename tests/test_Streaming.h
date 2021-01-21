#include "../UbiForm/Utilities/base64.h"

void doStuff(Component* c, std::iostream* fs){
    auto sendEndpoint = c->getSenderEndpointsByType("streamSender")->at(0);
    sendEndpoint->sendStream(*fs,2001);
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


    std::fstream fs;
    fs.open("TestManifests/IMG_7786.JPG", std::fstream::in|std::fstream::binary);
    if(!fs.good()){
        std::cerr << "Error with file" << std::endl;
    }
    std::thread t(doStuff,&sendComponent, &fs);
    std::string recvMessage;
    while(true) {
        auto message = recvEndpoint->receiveMessage();
        std::string r  = message->getString("bytes");
        recvMessage += r;
        if(r.empty()){
            break;
        }
    }
    std::cout << "Size: " << recvMessage.size() << std::endl;
    auto decode = base64_decode(recvMessage);
    std::cout << "Vector size: " << decode.size() << std::endl;
    std::fstream out;
    out.open("receive.jpg",std::fstream::out|std::fstream::binary);
    std::copy(decode.cbegin(),decode.cend(), std::ostream_iterator<unsigned char>(out));
    t.join();
}

