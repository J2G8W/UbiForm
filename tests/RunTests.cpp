#include "gtest/gtest.h"

#include "../UbiForm/test_ComponentManifest.h"
#include "../UbiForm/test_SocketMessage.h"
#include "../UbiForm/test_Component.h"
#include "../UbiForm/endpoints/test_EndpointSchema.h"
#include "../UbiForm/ResourceDiscovery/test_ResourceDiscoveryStore.h"
#include "../UbiForm/ResourceDiscovery/test_ComponentRepresentation.h"
#include "../UbiForm/ResourceDiscovery/test_ResourceDiscoveryConnEndpoint.h"
#include "../UbiForm/SystemSchemas/test_SystemSchemas.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
