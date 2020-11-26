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
        if (strcmp(argv[1], SENDER) == 0){
            Component sender;

            FILE* pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            sender.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.createNewPairEndpoint("v1", "UNIQUE_ID_2");
            std::shared_ptr<DataSenderEndpoint> senderEndpoint = sender.getSenderEndpoint("UNIQUE_ID_2");
            senderEndpoint->listenForConnection("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            SocketMessage s;
            for (int i = 0; i<10; i++) {
                s.addMember("temp", rand() % 100);
                s.addMember("msg", std::string("HELLO WORLD!"));
                std::vector<int> arrayValue = {rand()%10, rand() % 100, rand() % 1000};
                s.addMember("moreData",arrayValue);
                senderEndpoint->sendMessage(s);
            }

        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
    }
}

