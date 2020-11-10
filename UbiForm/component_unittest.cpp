#include "gtest/gtest.h"

#include "library.h"

#include <string>

TEST(ComponentManifest, GetNameTest){
    ComponentManifest testManifest(R"({"name":"TEST1"})");
    std::string expected_output ("TEST1");

    EXPECT_EQ(testManifest.getName(), expected_output);

}

