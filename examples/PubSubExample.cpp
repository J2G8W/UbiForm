
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

            FILE* pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            component.requestAndCreateConnection("subscriberExample",
                                                 "tcp://127.0.0.1:8000",
                                                 "publisherExample");
            component.requestAndCreateConnection("subscriberExample",
                                                 "tcp://127.0.0.1:8000",
                                                 "publisherExample");
            std::unique_ptr<SocketMessage> s;

            auto subscriberEndpoints = component.getReceiverEndpointsByType("subscriberExample");
            while(true){
                for (const auto& endpoint : *subscriberEndpoints) {
                    s = endpoint->receiveMessage();

                    std::string date = s->getString("date");
                    if (s->getBoolean("reverse")) {
                        std::cout << date << std::endl;
                    } else {
                        std::reverse(date.begin(), date.end());
                        std::cout << date << std::endl;
                    }
                }
            }

        }
        if (strcmp(argv[1], PUBLISHER) == 0){
            Component component;

            FILE* pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            std::cout << "MANIFEST SPECIFIED" << "\n";

            component.startBackgroundListen(8000);

            SocketMessage s;
            bool valid = true;
            auto publisherEndpoints = component.getSenderEndpointsByType("publisherExample");
            while(true) {
                if (!publisherEndpoints->empty()) {
                    s.addMember("reverse", valid);
                    valid = !valid;

                    char dateString[30];
                    time_t t = time(nullptr);
                    struct tm *p = localtime(&t);
                    strftime(dateString, 30, "%c", p);
                    s.addMember("date", std::string(dateString));

                    publisherEndpoints->at(0)->sendMessage(s);
                }
                sleep(1);
            }
        }
    }
    else{
        std::cerr << "Error usage is " << argv[0] << " " << PUBLISHER <<"|"<< SUBSCRIBER << "\n";
    }
}

