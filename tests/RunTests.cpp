#include "gtest/gtest.h"

#include "UnitTests/test_ComponentManifest.h"
#include "UnitTests/test_EndpointMessage.h"
#include "UnitTests/test_Component.h"
#include "UnitTests/test_EndpointSchema.h"
#include "UnitTests/test_ResourceDiscoveryStore.h"
#include "UnitTests/test_ComponentRepresentation.h"
#include "UnitTests/test_ResourceDiscoveryConnEndpoint.h"
#include "UnitTests/test_SystemSchemas.h"
#include "UnitTests/test_GetIPAddresses.h"

#include "IntegrationTests/test_EndpointCreation.h"
#include "IntegrationTests/test_Streaming.h"
#include "IntegrationTests/test_ReconfigurationIntegration.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
