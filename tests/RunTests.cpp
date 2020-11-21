#include "gtest/gtest.h"

#include "../UbiForm/test_ComponentManifest.h"
#include "../UbiForm/test_SocketMessage.h"
#include "../UbiForm/endpoints/test_EndpointSchema.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
