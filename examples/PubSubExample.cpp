
#include <algorithm>
#include "../include/UbiForm/Component.h"

#include <iomanip>
#include <nng/supplemental/util/platform.h>

#define SUBSCRIBER_COMPONENT "SUBSCRIBER"
#define PUBLISHER_COMPONENT "PUBLISHER"

int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], SUBSCRIBER_COMPONENT) == 0) {
            Component component("tcp://127.0.0.2");

            FILE *pFile = fopen("JsonFiles/SubscriberManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";

            component.getBackgroundRequester().requestRemoteListenThenDial(
                    "tcp://127.0.0.1", 8000, "subscriberExample",
                    "publisherExample");
            component.getBackgroundRequester().requestRemoteListenThenDial(
                    "tcp://127.0.0.1", 8000, "subscriberExample",
                    "publisherExample");
            std::unique_ptr<EndpointMessage> s;

            auto subscriberEndpoints = component.getEndpointsByType("subscriberExample");
            while (true) {
                for (const auto &endpoint : *subscriberEndpoints) {
                    s = component.castToDataReceiverEndpoint(endpoint)->receiveMessage();

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
        if (strcmp(argv[1], PUBLISHER_COMPONENT) == 0) {
            Component component("tcp://127.0.0.1");

            FILE *pFile = fopen("JsonFiles/PublisherManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            component.specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";

            component.startBackgroundListen(8000);

            EndpointMessage s;
            bool valid = true;
            auto publisherEndpoints = component.getEndpointsByType("publisherExample");
            while (true) {
                if (!publisherEndpoints->empty()) {
                    s.addMember("reverse", valid);
                    valid = !valid;

                    char dateString[30];
                    time_t t = time(nullptr);
                    struct tm *p = localtime(&t);
                    strftime(dateString, 30, "%c", p);
                    s.addMember("date", std::string(dateString));

                    component.castToDataSenderEndpoint(publisherEndpoints->at(0))->sendMessage(s);
                }
                nng_msleep(1000);
            }
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << PUBLISHER_COMPONENT << "|" << SUBSCRIBER_COMPONENT << "\n";
    }
}

