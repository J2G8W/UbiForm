#include "GetIPAddress.h"

#ifdef LINUX_BUILD

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdexcept>


std::vector<std::string> getLinuxIpAddresses() {
    std::vector<std::string> returnAddresses;

    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        throw std::logic_error("Unable to get ifadresses");
    }
    for (ifa = ifaddr, n = 0; ifa != nullptr; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == nullptr)
            continue;

        family = ifa->ifa_addr->sa_family;

        // We only care about IPv4 for the moment
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr,
                            sizeof(struct sockaddr_in),
                            host, NI_MAXHOST,
                            nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                throw std::logic_error("getnameinfo() failed: " + std::string(gai_strerror(s)));
            }
            returnAddresses.emplace_back(host);
        }
    }
    freeifaddrs(ifaddr);
    return returnAddresses;
}

#endif

std::vector<std::string> getIpAddresses() {
#ifdef LINUX_BUILD
    return getLinuxIpAddresses();
#endif
#ifdef WINDOWS_BUILD
    return getWindowsIpAddresses();
#endif
    std::vector<std::string> empty;
    return empty;
}

#ifdef WINDOWS_BUILD
#include <stdio.h>
#include <WinSock.h>
#pragma comment(lib, "wsock32.lib")
std::vector<std::string> getWindowsIpAddresses(){

    std::vector<std::string> returnVector;
    WORD wVersionRequested;
    WSADATA wsaData;
    char name[255];
    PHOSTENT hostinfo;
    wVersionRequested = MAKEWORD( 1, 1 );
    char *ip;

    if ( WSAStartup( wVersionRequested, &wsaData ) == 0 ){
        if( gethostname ( name, sizeof(name)) == 0){

            if((hostinfo = gethostbyname(name)) != NULL){
                int nCount = 0;
                while(hostinfo->h_addr_list[nCount])
                {
                 ip = inet_ntoa(*(
                      struct in_addr *)hostinfo->h_addr_list[nCount]);

                 returnVector.emplace_back(ip);
                 nCount++;
                }
            }
        }
    }
    
    return returnVector;
}
#endif