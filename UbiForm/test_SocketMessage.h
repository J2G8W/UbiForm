#include "gtest/gtest.h"

#include "SocketMessage.h"

TEST(SocketMessage, AddMember){
    SocketMessage socketMessage;
    socketMessage.addMember( "A", 42);
    socketMessage.addMember("B" , true);
    socketMessage.addMember("C", std::string{"HELLO"});

    EXPECT_STREQ(socketMessage.stringify(), R"({"A":42,"B":true,"C":"HELLO"})");
}

TEST(SocketMessage, OverwriteInteger){
    SocketMessage socketMessage;
    socketMessage.addMember("A", 42);
    socketMessage.addMember("A", 0);

    EXPECT_STREQ(socketMessage.stringify(), R"({"A":0})");
}

TEST(SocketMessage, OverwriteString){
    SocketMessage socketMessage;
    socketMessage.addMember("A", std::string("HELLO"));
    socketMessage.addMember("A", std::string("WORLD!"));

    EXPECT_STREQ(socketMessage.stringify(), R"({"A":"WORLD!"})");
}

