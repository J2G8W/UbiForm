#include "gtest/gtest.h"

#include "SocketMessage.h"

TEST(SocketMessage, AddInteger){
    SocketMessage socketMessage;
    std::string Attribute1 = "A";
    std::string Attribute2 = "C";
    socketMessage.addInteger(Attribute1, 42);
    socketMessage.addInteger(Attribute2, 0);

    EXPECT_STREQ(socketMessage.stringify(), R"({"A":42,"C":0})");
}

