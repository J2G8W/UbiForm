#include "gtest/gtest.h"

#include "ComponentManifest.h"

#include <string>

TEST(ComponentManifest, GetNameTest) {
    ComponentManifest testManifest(R"({"name":"TEST1"})");
    std::string expected_output("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);
}

TEST(ComponentManifest, StringifyTest) {
    const char *jsonString = R"({"name":"TEST1"})";
    ComponentManifest testManifest(jsonString);

    char *outputString = testManifest.stringify();
    EXPECT_STREQ(outputString, jsonString);
}

