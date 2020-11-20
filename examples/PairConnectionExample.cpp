#include <cstring>
#include <cstdlib>

#include "../UbiForm/Component.h"
#define RECEIVER "RECEIVER"
#define SENDER "SENDER"



int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], RECEIVER) == 0){
            Component receiver;

            receiver.specifyManifest(R"({"name":"RECEIVER"})");
            std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.createPairConnectionIncoming("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            std::unique_ptr<SocketMessage> s;
            for(int i =0; i< 5; i++){
                s = receiver.receiveMessage();
                std::cout << "Data: " << s->stringify() << "\n";
            }

        }
        if (strcmp(argv[1], SENDER) == 0){
            Component sender;

            FILE* pFile = fopen("JsonFiles/SenderManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            sender.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.createPairConnectionOutgoing("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            SocketMessage s;
            for (int i = 0; i<10; i++) {
                s.addMember("temp", rand() % 100);
                s.addMember("msg", std::string("HELLO"));
                sender.sendMessage(s);
            }

        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER <<"|"<<SENDER << "\n";
    }
}

