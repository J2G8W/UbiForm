#include "gtest/gtest.h"
#include "GetIPAddress.h"

TEST(IpAddress, Linux) {
    std::vector<std::string> addrs;
    ASSERT_NO_THROW(addrs = getLinuxIpAddresses());
    for (auto a : addrs) {
        std::cout << a << std::endl;
    }
}
