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

TEST(SocketMessage, RecursiveObject){
    // Our recursive Object
    auto *miniInput = new SocketMessage;
    miniInput->addMember("B", 100);

    // Our main object
    SocketMessage main;
    main.addMember("A",42);
    main.addMember("C", *miniInput);

    // Check main has in fact got the structure we expect
    EXPECT_EQ(main.stringify(), R"({"A":42,"C":{"B":100}})");

    // This tests that main is not still using memory allocated by miniInput - VERY IMPORTANT
    delete miniInput;

    // Get the recursive object back out of our message
    auto miniOutput = main.getCopyObject("C");
    // Mini is repopulated
    EXPECT_EQ(miniOutput->getInteger("B"), 100);
    EXPECT_EQ(main.getInteger("A"),42);

}

TEST(SocketMessage, AddMoveObject){
    SocketMessage sm;
    auto subSM = std::make_unique<SocketMessage>();
    subSM->addMember("HELLO",50);
    sm.addMoveObject("sub", std::move(subSM));

    ASSERT_EQ(subSM, nullptr);
    ASSERT_EQ(sm.stringify(), R"({"sub":{"HELLO":50}})");
}

TEST(SocketMessage, GetMoveObject){
    SocketMessage sm;
    auto subSM = std::make_unique<SocketMessage>();
    subSM->addMember("HELLO",50);
    sm.addMoveObject("sub", std::move(subSM));
    ASSERT_EQ(sm.stringify(), R"({"sub":{"HELLO":50}})");

    auto retSocketMessage = sm.getMoveObject("sub");
    ASSERT_EQ(retSocketMessage->getInteger("HELLO"),50);
    ASSERT_TRUE(sm.isNull("sub"));
}
TEST(SocketMessage, GetMoveArrayOfObjects){
    SocketMessage sm;
    auto subSm = new SocketMessage;
    subSm->addMember("HELLO",50);
    std::vector<SocketMessage *> lilVec = {subSm};
    sm.addMember("sub",lilVec);

    ASSERT_EQ(sm.stringify(), R"({"sub":[{"HELLO":50}]})");

    auto retSocketMessages = sm.getMoveArrayOfObjects("sub");
    ASSERT_EQ(retSocketMessages.size(), 1);
    ASSERT_EQ(retSocketMessages.at(0)->getInteger("HELLO"), 50);
    delete subSm;
}

TEST(SocketMessage, OverwriteInteger) {
    SocketMessage socketMessage;
    socketMessage.addMember("A", 42);
    EXPECT_EQ(socketMessage.getInteger("A") , 42);

    socketMessage.addMember("A", 7);
    EXPECT_EQ(socketMessage.stringify(), R"({"A":7})");
    EXPECT_EQ(socketMessage.getInteger("A") , 7);
}

TEST(SocketMessage, OverwriteString) {
    SocketMessage socketMessage;
    socketMessage.addMember("A", std::string("HELLO"));
    EXPECT_EQ(socketMessage.getString("A"), "HELLO");

    socketMessage.addMember("A", std::string("WORLD!"));
    EXPECT_EQ(socketMessage.stringify(), R"({"A":"WORLD!"})");
    EXPECT_EQ(socketMessage.getString("A"), "WORLD!");
}

TEST(SocketMessage, BadStringInput){
    EXPECT_ANY_THROW(new SocketMessage(R"({"HELLO":42)"));
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
    // Our main object to hold things
    SocketMessage main;
    // Our array to be of objects
    std::vector<SocketMessage*> inputObjectArray;
    for (int i =0; i<3; i++){
        inputObjectArray.push_back(new SocketMessage);
        inputObjectArray.back()->addMember("B", i);
    }
    main.addMember("A", inputObjectArray);
    // We check that our main object has the value we expect
    EXPECT_EQ(main.stringify(), R"({"A":[{"B":0},{"B":1},{"B":2}]})");

    // Due to memory safety we can delete these all good
    for (auto obj: inputObjectArray){
        delete obj;
    }

    // Get the array out again
    std::vector<std::unique_ptr<SocketMessage>> returnObjectArray = main.getArray<std::unique_ptr<SocketMessage>>("A");
    // Each object should have the values we put in
    EXPECT_EQ(returnObjectArray.at(0)->getInteger("B"), 0);
}

TEST(SocketMessage, AddMoveArrayOfObjects){
    SocketMessage main;
    // Our array to be of objects
    std::vector<std::unique_ptr<SocketMessage>> inputObjectArray;
    for (int i =0; i<3; i++){
        inputObjectArray.push_back(std::make_unique<SocketMessage>());
        inputObjectArray.back()->addMember("B", i);
    }
    main.addMoveArrayOfObjects("A", inputObjectArray);
    EXPECT_EQ(main.stringify(),R"({"A":[{"B":0},{"B":1},{"B":2}]})");
    EXPECT_EQ(inputObjectArray.at(0), nullptr);
}