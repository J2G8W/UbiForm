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

TEST(SocketMessage, ObjectTest){
    SocketMessage main;
    SocketMessage *mini = new SocketMessage;
    main.addMember("A",42);
    mini->addMember("B", 100);
    main.addMember("C", *mini);

    // Note that mini* is now rapidjson::null due to move semantics
    EXPECT_EQ(main.stringify(), R"({"A":42,"C":{"B":100}})");
    EXPECT_EQ(mini->stringify(), "null");

    mini = main.getObject("C");
    // Mini is repopulated and main/C becomes null
    EXPECT_EQ(mini->getInteger("B"), 100);
    EXPECT_EQ(main.stringify(), R"({"A":42,"C":null})");
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


TEST(SocketMessage, StringArray){
    SocketMessage socketMessage;
    std::vector<std::string> stringArray {"Hello","its","me","I've","been"};
    socketMessage.addMember("B",stringArray);
    EXPECT_EQ(socketMessage.stringify(), R"({"B":["Hello","its","me","I've","been"]})");
    EXPECT_EQ(socketMessage.getArray<std::string>("B"), stringArray);
}

TEST(SocketMessage, ObjectArray){
    SocketMessage main;
    std::vector<SocketMessage*> objectArray;
    objectArray.reserve(3);
    for (int i =0; i<3; i++){
        objectArray.push_back(new SocketMessage);
        objectArray.back()->addMember("B",10);
    }
    main.addMember("A",objectArray);
    EXPECT_EQ(objectArray.at(0)->stringify(), "null");
    EXPECT_EQ(main.stringify(), R"({"A":[{"B":10},{"B":10},{"B":10}]})");

    objectArray = main.getArray<SocketMessage *>("A");
    EXPECT_EQ(objectArray.at(0)->getInteger("B"), 10);
    EXPECT_EQ(main.stringify(), R"({"A":[null,null,null]})");
}