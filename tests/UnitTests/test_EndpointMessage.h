#include "gtest/gtest.h"

#include <vector>

#include "../../include/UbiForm/EndpointMessage.h"

TEST(EndpointMessageTest, Stringify) {
    std::string jsonString = R"({"temperature":42,"value":"HELLO WORLD!"})";
    EndpointMessage endpointMessage(jsonString.c_str());
    EXPECT_EQ(jsonString, endpointMessage.stringify());

}

TEST(EndpointMessageTest, AddMember) {
    EndpointMessage endpointMessage;
    endpointMessage.addMember("A", 42);
    endpointMessage.addMember("B", true);
    endpointMessage.addMember("C", std::string{"HELLO"});

    EXPECT_EQ(endpointMessage.stringify(), R"({"A":42,"B":true,"C":"HELLO"})");
}

TEST(EndpointMessageTest, RecursiveObject) {
    // Our recursive Object
    auto *miniInput = new EndpointMessage;
    miniInput->addMember("B", 100);

    // Our main object
    EndpointMessage main;
    main.addMember("A", 42);
    main.addMember("C", *miniInput);

    // Check main has in fact got the structure we expect
    EXPECT_EQ(main.stringify(), R"({"A":42,"C":{"B":100}})");

    // This tests that main is not still using memory allocated by miniInput - VERY IMPORTANT
    delete miniInput;

    // Get the recursive object back out of our message
    auto miniOutput = main.getCopyObject("C");
    // Mini is repopulated
    EXPECT_EQ(miniOutput->getInteger("B"), 100);
    EXPECT_EQ(main.getInteger("A"), 42);

}

TEST(EndpointMessageTest, AddMoveObject) {
    EndpointMessage sm;
    auto subSM = std::make_unique<EndpointMessage>();
    subSM->addMember("HELLO", 50);
    sm.addMoveObject("sub", std::move(subSM));

    ASSERT_EQ(subSM, nullptr);
    ASSERT_EQ(sm.stringify(), R"({"sub":{"HELLO":50}})");
}

TEST(EndpointMessageTest, GetMoveObject) {
    EndpointMessage sm;
    auto subSM = std::make_unique<EndpointMessage>();
    subSM->addMember("HELLO", 50);
    sm.addMoveObject("sub", std::move(subSM));
    ASSERT_EQ(sm.stringify(), R"({"sub":{"HELLO":50}})");

    auto retEndpointMessageTest = sm.getMoveObject("sub");
    ASSERT_EQ(retEndpointMessageTest->getInteger("HELLO"), 50);
    ASSERT_TRUE(sm.isNull("sub"));
}

TEST(EndpointMessageTest, GetMoveArrayOfObjects) {
    EndpointMessage sm;
    auto subSm = new EndpointMessage;
    subSm->addMember("HELLO", 50);
    std::vector<EndpointMessage *> lilVec = {subSm};
    sm.addMember("sub", lilVec);

    ASSERT_EQ(sm.stringify(), R"({"sub":[{"HELLO":50}]})");

    auto retEndpointMessageTests = sm.getMoveArrayOfObjects("sub");
    ASSERT_EQ(retEndpointMessageTests.size(), 1);
    ASSERT_EQ(retEndpointMessageTests.at(0)->getInteger("HELLO"), 50);
    delete subSm;
}

TEST(EndpointMessageTest, OverwriteInteger) {
    EndpointMessage endpointMessage;
    endpointMessage.addMember("A", 42);
    EXPECT_EQ(endpointMessage.getInteger("A"), 42);

    endpointMessage.addMember("A", 7);
    EXPECT_EQ(endpointMessage.stringify(), R"({"A":7})");
    EXPECT_EQ(endpointMessage.getInteger("A"), 7);
}

TEST(EndpointMessageTest, OverwriteString) {
    EndpointMessage endpointMessage;
    endpointMessage.addMember("A", std::string("HELLO"));
    EXPECT_EQ(endpointMessage.getString("A"), "HELLO");

    endpointMessage.addMember("A", std::string("WORLD!"));
    EXPECT_EQ(endpointMessage.stringify(), R"({"A":"WORLD!"})");
    EXPECT_EQ(endpointMessage.getString("A"), "WORLD!");
}

TEST(EndpointMessageTest, BadStringInput) {
    EXPECT_ANY_THROW(new EndpointMessage(R"({"HELLO":42)"));
}


TEST(EndpointMessageTest, IntegerArray) {
    EndpointMessage endpointMessage;
    std::vector<int> intArray{1, 2, 3, 4};
    endpointMessage.addMember("A", intArray);

    EXPECT_EQ(endpointMessage.stringify(), R"({"A":[1,2,3,4]})");
    EXPECT_EQ(endpointMessage.getArray<int>("A"), intArray);

}

TEST(EndpointMessageTest, BooleanArray) {

    EndpointMessage endpointMessage;
    std::vector<bool> boolArray{true, false, true};
    endpointMessage.addMember("B", boolArray);
    EXPECT_EQ(endpointMessage.stringify(), R"({"B":[true,false,true]})");
    EXPECT_EQ(endpointMessage.getArray<bool>("B"), boolArray);
}


TEST(EndpointMessageTest, StringArray) {
    EndpointMessage endpointMessage;
    std::vector<std::string> stringArray{"Hello", "its", "me", "I've", "been"};
    endpointMessage.addMember("B", stringArray);
    EXPECT_EQ(endpointMessage.stringify(), R"({"B":["Hello","its","me","I've","been"]})");
    EXPECT_EQ(endpointMessage.getArray<std::string>("B"), stringArray);
}

TEST(EndpointMessageTest, ObjectArray) {
    // Our main object to hold things
    EndpointMessage main;
    // Our array to be of objects
    std::vector<EndpointMessage *> inputObjectArray;
    for (int i = 0; i < 3; i++) {
        inputObjectArray.push_back(new EndpointMessage);
        inputObjectArray.back()->addMember("B", i);
    }
    main.addMember("A", inputObjectArray);
    // We check that our main object has the value we expect
    EXPECT_EQ(main.stringify(), R"({"A":[{"B":0},{"B":1},{"B":2}]})");

    // Due to memory safety we can delete these all good
    for (auto obj: inputObjectArray) {
        delete obj;
    }

    // Get the array out again
    std::vector<std::unique_ptr<EndpointMessage>> returnObjectArray = main.getArray<std::unique_ptr<EndpointMessage>>("A");
    // Each object should have the values we put in
    EXPECT_EQ(returnObjectArray.at(0)->getInteger("B"), 0);
}

TEST(EndpointMessageTest, AddMoveArrayOfObjects) {
    EndpointMessage main;
    // Our array to be of objects
    std::vector<std::unique_ptr<EndpointMessage>> inputObjectArray;
    for (int i = 0; i < 3; i++) {
        inputObjectArray.push_back(std::make_unique<EndpointMessage>());
        inputObjectArray.back()->addMember("B", i);
    }
    main.addMoveArrayOfObjects("A", inputObjectArray);
    EXPECT_EQ(main.stringify(), R"({"A":[{"B":0},{"B":1},{"B":2}]})");
    EXPECT_EQ(inputObjectArray.at(0), nullptr);
}