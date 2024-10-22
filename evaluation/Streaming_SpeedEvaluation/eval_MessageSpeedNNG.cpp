

#include <nng/nng.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <nng/supplemental/util/platform.h>
#include <nng/protocol/pair0/pair.h>
#include <chrono>
#include <vector>
#include <sstream>


#define RECEIVER "RECEIVER"
#define SENDER "SENDER"
#define MESSAGE_SIZE 100000
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
    if (argc >= 3 && strcmp(argv[1], RECEIVER) == 0) {
        std::vector<TimingData> timings(NUM_TESTS);
        int rv;
        nng_socket* recv_socket = new nng_socket;

        if ((rv = nng_pair0_open(recv_socket)) != 0) {
            nng_err(rv, "Open pair");
        }
        if ((rv = nng_dial(*recv_socket, argv[2], nullptr, 0)) != 0) {
            nng_err(rv, "Dialing " + std::string(argv[2]) + " for a pair connection");
        }

        std::ostringstream output;
        for(auto& t : timings){
            t.startTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            size_t bytesReceived = 0;
            while(true) {
                char *buffer = nullptr;
                size_t sz;
                if (nng_recv(*recv_socket, &buffer, &sz, NNG_FLAG_ALLOC) == 0) {
                    if (sz == 4 && std::string(buffer) == "end"){
                        break;
                    }
                    bytesReceived += sz;
                    // Write to a stream for comparison to UbiForm
                    output.write(buffer,sz);
                    nng_free(buffer, sz);
                }
            }
            t.endTime = std::chrono::high_resolution_clock::now().time_since_epoch();
            t.bytesReceived = bytesReceived;
            nng_msleep(500);
        }


        std::ofstream results;
        results.open("messaging_nng_results.csv",std::fstream::out | std::fstream::app);

        for(auto &t : timings){
            t.duration = t.endTime - t.startTime;
            results << t.duration.count() << "," << t.bytesReceived << "," << MESSAGE_SIZE << "\n";
        }
        results.close();

    } else if (argc >= 3 && strcmp(argv[1], SENDER) == 0) {
        int rv;
        nng_socket* sender_socket = new nng_socket;
        if ((rv = nng_pair0_open(sender_socket)) != 0) {
            nng_err(rv, "Open pair");
        }
        if ((rv = nng_listen(*sender_socket, "tcp://*:9000", nullptr, 0)) != 0) {
            nng_err(rv, "Dialing for a pair connection");
        }

        std::ifstream infile(argv[2], std::fstream::binary | std::fstream::in);
        for(int i = 0; i < NUM_TESTS; i++) {

            char *buffer = static_cast<char *>(calloc(MESSAGE_SIZE, sizeof(char)));

            while (!infile.eof()) {
                infile.read(buffer, MESSAGE_SIZE);
                if ((rv = nng_send(*sender_socket, (void *) buffer, infile.gcount(), 0)) != 0) {
                    nng_err(rv, "nng_send");
                }
            }
            const char* endMsg = "end";
            nng_send(*sender_socket, (void*)endMsg, 4,0);
            infile.clear();
            infile.seekg(0, std::ifstream::beg);
        }
        nng_msleep(1000);
    } else {
        std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << " SENDER_ADDRESS\n";
        std::cerr << argv[0] << " " << SENDER << " fileLocation" << std::endl;
    }
}
