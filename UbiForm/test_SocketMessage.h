#include "gtest/gtest.h"

#include <vector>

#include "SocketMessage.h"

TEST(SocketMessage, Stringify){
    std::string jsonString = R"({"temperature":42,"value":"HELLO WORLD!"})";
    SocketMessage socketMessage(jsonString.c_str());
    EXPECT_EQ(jsonString, socketMessage.stringify());

}

TEST(SocketMessage, AddMember) {
    SocketMessage socketMessage;
    socketMessage.addMember("A", 42);
    socketMessage.addMember("B", true);
    socketMessage.addMember("C", std::string{"HELLO"});

    EXPECT_EQ(socketMessage.stringify(), R"({"A":42,"B":true,"C":"HELLO"})");
}

TEST(SocketMessage, AddArray){
    const std::vector<int> inputVector = {1,2,3,4};

    SocketMessage socketMessage;
    socketMessage.addMember<int>("A",inputVector);

    EXPECT_EQ(socketMessage.stringify(), R"({"A":[1,2,3,4]})");

}

TEST(SocketMessage, OverwriteInteger) {
    SocketMessage socketMessage;
    socketMessage.addMember("A", 42);
    socketMessage.addMember("A", 0);

    EXPECT_EQ(socketMessage.stringify(), R"({"A":0})");
}

TEST(SocketMessage, OverwriteString) {
    SocketMessage socketMessage;
    socketMessage.addMember("A", std::string("HELLO"));
    socketMessage.addMember("A", std::string("WORLD!"));

    EXPECT_EQ(socketMessage.stringify(), R"({"A":"WORLD!"})");
}

TEST(SocketMessage, BadStringInput){
    EXPECT_ANY_THROW(new SocketMessage(R"({"HELLO":42)"));
}