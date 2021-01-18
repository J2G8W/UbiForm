#include "gtest/gtest.h"
#include "GetIPAddress.h"

TEST(IpAddress, General) {
    std::vector<std::string> addrs;
    ASSERT_NO_THROW(addrs = getIpAddresses());
    for (auto a : addrs) {
        std::cout << a << std::endl;
    }
}
