#ifndef UBIFORM_GETIPADDRESS_H
#define UBIFORM_GETIPADDRESS_H

#ifdef __linux__
#define LINUX_BUILD
#endif
#ifdef _WIND32
#define WINDOWS_BUILD
#endif
#ifdef __ANDROID__
#define ANDROID_BUILD
#endif


#include <vector>
#include <string>

#ifdef LINUX_BUILD
/**
 * For Linux systems only, creates an array of all the IPv4 addresses that the device can be considered to listen on
 * @return Array of IPv4 addresses
 */
std::vector<std::string> getLinuxIpAddresses();
#endif

#ifdef WINDOWS_BUILD
std::vector<std::string> getWindowsIpAddresses();
#endif

std::vector<std::string> getIpAddresses();

#endif //UBIFORM_GETIPADDRESS_H
