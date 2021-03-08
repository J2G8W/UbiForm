

#include <nng/nng.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <nng/supplemental/util/platform.h>
#include <nng/protocol/pair0/pair.h>
#include <chrono>
#include <vector>


#define RECEIVER "RECEIVER"
#define SENDER "SENDER"
#define MESSAGE_SIZE 10000
#define NUM_TESTS 5

void nng_err(int rv, const std::string& function_name){
    std::cerr << "NNG error with function: " << function_name << "\nError text: " << nng_strerror(rv) << std::endl;
    exit(1);
}

struct TimingData{
    std::chrono::duration<int64_t, std::nano> startTime;
    std::chrono::duration<int64_t, std::nano> endTime;
    std::chrono::duration<int64_t, std::nano> duration;
    size_t bytesReceived;
};

int main(int argc, char **argv){
    if (strcmp(argv[1], RECEIVER) == 0) {
        std::vector<TimingData> timings(NUM_TESTS);
        int rv;
        nng_socket* recv_socket = new nng_socket;

        if ((rv = nng_pair0_open(recv_socket)) != 0) {
            nng_err(rv, "Open pair");
        }
        if ((rv = nng_dial(*recv_socket, argv[2], nullptr, 0)) != 0) {
            nng_err(rv, "Dialing " + std::string(argv[2]) + " for a pair connection");
        }

        for(auto t : timings){
            t.startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            size_t bytesReceived = 0;
            while(true) {
                char *buffer = nullptr;
                size_t sz;
                if ((rv = nng_recv(*recv_socket, &buffer, &sz, NNG_FLAG_ALLOC)) == 0) {
                    if (sz == 0){
                        break;
                    }
                    bytesReceived += sz;
                    nng_free(buffer, sz);
                }
            }
            t.endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            t.bytesReceived = bytesReceived;
            nng_msleep(100);
        }


        std::ofstream results;
        results.open("messaging_nng_results.csv",std::fstream::out | std::fstream::app);

        for(auto &t : timings){
            t.duration = t.endTime - t.startTime;
            results << t.duration.count() << "," << t.bytesReceived << "," << MESSAGE_SIZE << "\n";
        }
        results.close();

    } else if (strcmp(argv[1], SENDER) == 0) {
        int rv;
        nng_socket* sender_socket = new nng_socket;
        if ((rv = nng_pair0_open(sender_socket)) != 0) {
            nng_err(rv, "Open pair");
        }
        if ((rv = nng_listen(*sender_socket, "tcp://*:9000", nullptr, 0)) != 0) {
            nng_err(rv, "Dialing for a pair connection");
        }

        for(int i = 0; i < NUM_TESTS; i++) {
            std::ifstream infile(argv[2], std::fstream::binary | std::fstream::in);

            size_t msg_size = MESSAGE_SIZE;
            char *buffer = static_cast<char *>(calloc(msg_size, sizeof(char)));

            while (!infile.eof()) {
                infile.read(buffer, msg_size);
                if ((rv = nng_send(*sender_socket, (void *) buffer, infile.gcount(), 0)) != 0) {
                    nng_err(rv, "nng_send");
                }
            }
            nng_send(*sender_socket, buffer, 0,0);
        }

        nng_msleep(1000);
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}
