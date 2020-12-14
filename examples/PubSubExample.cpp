
#include <algorithm>
#include "../UbiForm/Component.h"

#include <chrono>
#include <iomanip>

#define SUBSCRIBER "SUBSCRIBER"
#define PUBLISHER "PUBLISHER"

int main(int argc, char ** argv){
    if (argc >= 2){
        if (strcmp(argv[1], SUBSCRIBER) == 0){
            Component component;

            FILE* pFile = fopen("JsonFiles/ReceiverManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            component.createNewSubscriberEndpoint("v1", "Sub1");
            std::shared_ptr<DataReceiverEndpoint> subscriber_endpoint = component.getReceiverEndpointById("Sub1");
            subscriber_endpoint->dialConnection("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            std::unique_ptr<SocketMessage> s;
            while(true){
                s = subscriber_endpoint->receiveMessage();

                std::string date = s->getString("date");
                if (s->getBoolean("reverse")) {
                    std::cout << date <<std::endl;
                }else{
                    std::reverse(date.begin(),date.end());
                    std::cout << date << std::endl;
                }
            }

        }
        if (strcmp(argv[1], PUBLISHER) == 0){
            Component component;

            FILE* pFile = fopen("JsonFiles/SenderManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            component.createNewPublisherEndpoint("v1", "Pub1");
            std::shared_ptr<DataSenderEndpoint> publisher_endpoint = component.getSenderEndpointById("Pub1");
            publisher_endpoint->listenForConnection("tcp://127.0.0.1:8000");
            std::cout << "CONNECTION MADE" << "\n";

            SocketMessage s;
            bool valid = true;
            for(int i = 0; i<10; i++) {
                s.addMember("reverse", valid);
                valid = !valid;


                char dateString[30];

                time_t t = time(nullptr);
                struct tm * p = localtime(&t);

                strftime(dateString, 30, "%c", p);
                s.addMember("date",std::string(dateString));

                publisher_endpoint->sendMessage(s);
                sleep(1);
            }



        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << PUBLISHER <<"|"<< SUBSCRIBER << "\n";
    }
}

