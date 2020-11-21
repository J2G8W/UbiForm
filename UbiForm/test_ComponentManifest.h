#include "gtest/gtest.h"

#include "ComponentManifest.h"

#include <string>

TEST(ComponentManifest, GetNameTest) {
    ComponentManifest testManifest(R"({"name":"TEST1","schema":{}})");
    std::string expected_output("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);
}

TEST(ComponentManifest, StringifyTest) {
    const char *jsonString = R"({"name":"TEST1","schema":{}})";
    ComponentManifest testManifest(jsonString);

    EXPECT_EQ(testManifest.stringify(), std::string(jsonString));
}

TEST(ComponentManifest, NoSchema){
    const char *jsonString = R"({"name":"TEST1"})";
    ASSERT_ANY_THROW(new ComponentManifest(jsonString));
}

TEST(ComponentManifest, MalformedManifest){
    const char *jsonString = R"({"name""TEST1")";
    ASSERT_ANY_THROW(new ComponentManifest(jsonString));
}
