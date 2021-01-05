#ifndef UBIFORM_BACKGROUNDLISTENER_H
#define UBIFORM_BACKGROUNDLISTENER_H

#include <thread>
#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include "../SystemSchemas/SystemSchemas.h"

class Component;
class BackgroundListener {
private:
    std::thread backgroundThread;
    std::string backgroundListenAddress;
    nng_socket backgroundSocket;
    Component * component;
    SystemSchemas & systemSchemas;

public:
    BackgroundListener(Component * c, SystemSchemas & ss) : backgroundSocket(), component(c), systemSchemas(ss){
        // Create the background socket;
        int rv;
        if ((rv = nng_rep0_open(&backgroundSocket)) != 0) {
            throw NngError(rv, "Opening background socket");
        }
    }

    void startBackgroundListen(const std::string& listenAddress);

    static void backgroundListen(BackgroundListener *backgroundListener);

    std::string getBackgroundListenAddress(){return backgroundListenAddress;}

    ~BackgroundListener();
};


#endif //UBIFORM_BACKGROUNDLISTENER_H
