#include <cstring>
#include <cstdlib>

#include "../UbiForm/Component.h"
#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


struct AsyncExtraInfo{
    std::shared_ptr<DataReceiverEndpoint> receiverEndpoint;
    int counter =0;

    explicit AsyncExtraInfo(std::shared_ptr<DataReceiverEndpoint> rE): receiverEndpoint(rE){}
} typedef AsyncExtraInfo;

void testCallback(SocketMessage * sm, void* moreData){
    std::cout << sm->getString("msg") << "   SUCCESS" << std::endl ;
    auto *extraInfo = static_cast<AsyncExtraInfo *>(moreData);
    extraInfo->counter += 1;
    if (extraInfo-> counter < 5) {
        extraInfo->receiverEndpoint->asyncReceiveMessage(testCallback, extraInfo);
    }else{
        delete extraInfo;
    }
}

int main(int argc, char ** argv){
    Component receiver;

    FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
    if (pFile == nullptr) perror("ERROR");
    receiver.specifyManifest(pFile);
    fclose(pFile);

    std::cout << "MANIFEST SPECIFIED" << "\n";

    receiver.createNewPairEndpoint("v1", "UNIQUE_ID_1");
    std::shared_ptr<DataReceiverEndpoint> receiverEndpoint = receiver.getReceiverEndpoint("UNIQUE_ID_1");
    receiverEndpoint->dialConnection("tcp://127.0.0.1:8000");
    std::cout << "CONNECTION MADE" << "\n";

    std::unique_ptr<SocketMessage> s;

    receiverEndpoint->asyncReceiveMessage(testCallback, new AsyncExtraInfo(receiverEndpoint));
    while(true){
        sleep(5);
    }

}

