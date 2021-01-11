#ifndef UBIFORM_GETIPADDRESS_H
#define UBIFORM_GETIPADDRESS_H

#include <vector>
#include <string>

/**
 * For Linux systems only, creates an array of all the IPv4 addresses that the device can be considered to listen on
 * @return Array of IPv4 addresses
 */
std::vector<std::string> getLinuxIpAddresses();

#endif //UBIFORM_GETIPADDRESS_H
