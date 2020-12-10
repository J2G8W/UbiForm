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

TEST(SocketMessage, BasicGetters){
    SocketMessage socketMessage;
    socketMessage.addMember("A", 42);
    EXPECT_EQ(socketMessage.getInteger("A") , 42);
    socketMessage.addMember("B", std::string("HELLO"));
    EXPECT_EQ(socketMessage.getString("B"), "HELLO");
}

TEST(SocketMessage, IntegerArray){
    SocketMessage socketMessage;
    std::vector<int> intArray {1, 2, 3, 4};
    socketMessage.addMember("A", intArray);

    EXPECT_EQ(socketMessage.stringify(), R"({"A":[1,2,3,4]})");
    EXPECT_EQ(socketMessage.getArray<int>("A"), intArray);

}

TEST(SocketMessage, BooleanArray){

    SocketMessage socketMessage;
    std::vector<bool> boolArray {true,false,true};
    socketMessage.addMember("B", boolArray);
    EXPECT_EQ(socketMessage.stringify(), R"({"B":[true,false,true]})");
    EXPECT_EQ(socketMessage.getArray<bool>("B"), boolArray);
}


TEST(SocketMessage, StringArrayTests){
    SocketMessage socketMessage;
    std::vector<std::string> stringArray {"Hello","its","me","I've","been"};
    socketMessage.addMember("B",stringArray);
    EXPECT_EQ(socketMessage.stringify(), R"({"B":["Hello","its","me","I've","been"]})");
    EXPECT_EQ(socketMessage.getArray<std::string>("B"), stringArray);
}