#include <iostream>

// For the moment we use the standard NNG function to quit
void fatal(const char *func, int rv){
    std::cerr << "Problem with function: " << func << "\nError text: " << nng_strerror(rv) <<"\n";
    exit(1);
}