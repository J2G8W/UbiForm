#include <nng/nng.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <nng/supplemental/util/platform.h>


#define RECEIVER "RECEIVER"
#define SENDER "SENDER"

int main(int argc, char **argv){
    if (strcmp(argv[1], RECEIVER) == 0) {
        nng_stream_listener* listener;
        nng_stream_listener_alloc(&listener, "tcp://*:9000");
        nng_stream_listener_listen(listener);

        nng_aio* aio = nullptr;
        nng_aio_alloc(&aio, nullptr, nullptr);
        nng_stream_listener_accept(listener, aio);

        nng_stream* stream = static_cast<nng_stream *>(nng_aio_get_output(aio, 0));

        size_t length = 100000;
        char* buffer = static_cast<char *>(calloc(length, sizeof(char)));
        nng_iov* iov = new nng_iov;
        iov->iov_buf = buffer;
        iov->iov_len = length;
        nng_aio_set_iov(aio,1,iov);

        nng_stream_recv(stream, aio);

        nng_msleep(4000);
        std::cout << buffer << std::endl;

    } else if (strcmp(argv[1], SENDER) == 0) {
        nng_stream_dialer* dialer = nullptr;
        nng_stream_dialer_alloc(&dialer, argv[2]);

        nng_aio* aio = nullptr;
        nng_aio_alloc(&aio, nullptr, nullptr);
        nng_stream_dialer_dial(dialer, aio);

        nng_stream* stream = static_cast<nng_stream *>(nng_aio_get_output(aio, 0));

        std::ifstream infile(argv[3]);

        //get length of file
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);

        char* buffer = static_cast<char *>(calloc(length, sizeof(char)));

        //read file
        infile.read(buffer, length);

        nng_iov* iov = new nng_iov;
        iov->iov_buf = buffer;
        iov->iov_len = length;
        nng_aio_set_iov(aio,1,iov);


        nng_stream_send(stream, aio);

        nng_msleep(4000);
    } else {
    std::cerr << "Error usage is " << argv[0] << " " << RECEIVER << "SENDER_ADDRESS\n";
    std::cerr << argv[0] << " " << SENDER << " [fileLocation]" << std::endl;
    }
}
