#include "GetIPAddress.h"

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
                throw std::logic_error("getnameinfo() failed: "+ std::string(gai_strerror(s)));
            }
            returnAddresses.emplace_back(host);
        }
    }
    freeifaddrs(ifaddr);
    return returnAddresses;
}
