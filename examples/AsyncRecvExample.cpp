#include <cstring>
#include <cstdlib>

#include "../UbiForm/Component.h"
#define RECEIVER "RECEIVER"
#define SENDER "SENDER"


void testCallback(SocketMessage * sm){
    std::cout << sm->getString("msg") << "   SUCCESS" << std::endl ;
}

int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], RECEIVER) == 0){
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

            receiverEndpoint->asyncReceiveMessage(testCallback);
            while(true){
                sleep(5);
            }
        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
    }
}

