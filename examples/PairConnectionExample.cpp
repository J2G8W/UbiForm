#include <cstring>

#include "../include/UbiForm/Component.h"
#include <nng/supplemental/util/platform.h>

#define RECEIVER "RECEIVER"
#define SENDER "SENDER"
#define RECEIVER_AIO "AIO"


struct SimpleData{
    Component * component;
    Endpoint* endpoint;
};
void simpleCallback(EndpointMessage *sm, void *data) {
    auto extraData = static_cast<SimpleData *>(data);
    std::cout << sm->getInteger("temp") << std::endl;
    auto endpoint = extraData->component->castToDataReceiverEndpoint(extraData->endpoint);
    endpoint->asyncReceiveMessage(simpleCallback, extraData);
}


int main(int argc, char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], RECEIVER) == 0) {
            Component receiver("tcp://127.0.0.2");
            FILE *pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            receiver.specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    "tcp://127.0.0.1", 8000, "pairExample",
                    "pairExample");
            auto endpoints = receiver.getEndpointsByType("pairExample");
            while (true) {
                for (const auto &e: *endpoints) {
                    auto msg = receiver.castToDataReceiverEndpoint(e)->receiveMessage();
                    std::cout << msg->getInteger("temp") << std::endl;
                }
            }

        } else if (strcmp(argv[1], RECEIVER_AIO) == 0) {
            Component receiver("tcp://127.0.0.2");
            FILE *pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            receiver.specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";

            receiver.getBackgroundRequester().requestRemoteListenThenDial(
                    "tcp://127.0.0.1", 8000, "pairExample",
                    "pairExample");
            auto endpoints = receiver.getEndpointsByType("pairExample");
            for (const auto& e: *endpoints) {
                auto* sd = new SimpleData;
                sd->component = &receiver;
                sd->endpoint = e.get();
                receiver.castToDataReceiverEndpoint(e)->asyncReceiveMessage(simpleCallback, sd);
            }
            while (true) {
                nng_msleep(1000);
            }

        } else if (strcmp(argv[1], SENDER) == 0) {
            Component sender("tcp://127.0.0.1");

            FILE *pFile = fopen("JsonFiles/PairManifest1.json", "r");
            if (pFile == nullptr) perror("ERROR");
            sender.specifyManifest(pFile);
            fclose(pFile);

            if(VIEW_STD_OUTPUT) std::cout << "MANIFEST SPECIFIED" << "\n";

            sender.startBackgroundListen(8000);
            auto endpointVector = sender.getEndpointsByType("pairExample");
            int i = 0;
            while (true) {
                nng_msleep(1000);
                try {
                    EndpointMessage sm;
                    sm.addMember("temp", i++);
                    sm.addMember("msg", std::string("HELLO WORLD!"));
                    for (const auto &e : *endpointVector) {
                        sender.castToDataSenderEndpoint(e)->asyncSendMessage(sm);
                    }

                } catch (std::out_of_range &e) {
                    std::cout << "ERROR" << std::endl;
                    // IGNORED
                }
            }

        } else {
            std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
        }
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "|" << SENDER << "\n";
    }
}