#include "gtest/gtest.h"

#include "SocketMessage.h"

TEST(SocketMessage, AddInteger){
    SocketMessage socketMessage;
    socketMessage.addMember( "A", 42);
    socketMessage.addMember("B" , true);
    socketMessage.addMember("C", std::string{"HELLO"});

    EXPECT_STREQ(socketMessage.stringify(), R"({"A":42,"B":true,"C":"HELLO"})");
}

