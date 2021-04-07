#include "gtest/gtest.h"
#include "../../UbiForm/Utilities/GetIPAddress.h"

TEST(IpAddress, General) {
    std::vector<std::string> addrs;
    ASSERT_NO_THROW(addrs = getIpAddresses());
}
