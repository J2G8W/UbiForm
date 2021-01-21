cmake_minimum_required(VERSION 3.17)
project(UbiForm)

set(CMAKE_CXX_STANDARD 14)

# Add pthread library so NNG can build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Set options whether we build tests and examples
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_EXAMPLES "Build examples programs" OFF)

# Setup nng library
include_directories(nng-master/include)
add_subdirectory(nng-master)

# Setup rapidjson
ADD_DEFINITIONS(-DRAPIDJSON_HAS_STDSTRING)
set(rapidjson_dir ${CMAKE_CURRENT_SOURCE_DIR}/../libs/rapidjson)

include_directories(${rapidjson_dir}/include)


set(endpointFiles ../include/UbiForm/Endpoints/DataReceiverEndpoint.h ../UbiForm/Endpoints/DataReceiverEndpoint.cpp
        ../include/UbiForm/Endpoints/DataSenderEndpoint.h ../UbiForm/Endpoints/DataSenderEndpoint.cpp
        ../include/UbiForm/Endpoints/PairEndpoint.h ../UbiForm/Endpoints/PairEndpoint.cpp
        ../UbiForm/Endpoints/SubscriberEndpoint.cpp ../include/UbiForm/Endpoints/SubscriberEndpoint.h
        ../UbiForm/Endpoints/PublisherEndpoint.cpp ../include/UbiForm/Endpoints/PublisherEndpoint.h
		../UbiForm/Endpoints/RequestEndpoint.cpp ../include/UbiForm/Endpoints/RequestEndpoint.h
		../UbiForm/Endpoints/ReplyEndpoint.cpp ../include/UbiForm/Endpoints/ReplyEndpoint.h)

set(reconfigEndpoints ../include/UbiForm/ReconfigurationEndpoints/BackgroundListener.h ../UbiForm/ReconfigEndpoints/BackgroundListener.cpp
		../UbiForm/ReconfigEndpoints/BackgroundRequester.cpp ../include/UbiForm/ReconfigurationEndpoints/BackgroundRequester.h)

set(resourceDiscovery ../UbiForm/ResourceDiscovery/ResourceDiscoveryHubEndpoint.cpp ../include/UbiForm/ResourceDiscovery/ResourceDiscoveryHubEndpoint.h
        ../UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.cpp ../include/UbiForm/ResourceDiscovery/ResourceDiscoveryConnEndpoint.h
		../include/UbiForm/ResourceDiscovery/ComponentRepresentation.h ../UbiForm/ResourceDiscovery/ComponentRepresentation.cpp
        ../UbiForm/ResourceDiscovery/ResourceDiscoveryStore.cpp ../include/UbiForm/ResourceDiscovery/ResourceDiscoveryStore.h)

set(schemaFiles ../include/UbiForm/SchemaRepresentation/EndpointSchema.h ../UbiForm/SchemaRepresentation/EndpointSchema.cpp
        ../UbiForm/SystemSchemas/SystemSchemas.h ../UbiForm/SystemSchemas/SystemSchemas.cpp
        ../UbiForm/SchemaRepresentation/GenericSchema.cpp ../UbiForm/SchemaRepresentation/GenericSchema.h)

set(utilityFiles ../UbiForm/Utilities/UtilityFunctions.h ../UbiForm/Utilities/UtilityFunctions.cpp
		../include/UbiForm/Utilities/SystemEnums.h ../UbiForm/Utilities/SystemEnums.cpp
		../UbiForm/Utilities/GetIPAddress.cpp ../UbiForm/Utilities/GetIPAddress.h ../UbiForm/Utilities/base64.h ../UbiForm/Utilities/base64.cpp)

set(componentFiles ../include/UbiForm/ComponentManifest.h ../UbiForm/ComponentManifest.cpp
		../include/UbiForm/Component.h ../UbiForm/Component.cpp
		../include/UbiForm/SocketMessage.h ../UbiForm/SocketMessage.cpp)


# Create library
file(COPY ../UbiForm/SystemSchemas DESTINATION .)
add_library(UbiForm ${endpointFiles} ${reconfigEndpoints} ${resourceDiscovery} ${schemaFiles} ${utilityFiles} ${componentFiles})
target_link_libraries(UbiForm nng)


# Create tests
if (BUILD_TESTS)
	#We use googletest for our testing. This downloads and sets it up using CmakeList.txt.in
	# Download and unpack googletest at configure time
	configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
		message(FATAL_ERROR "CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
	if(result)
		message(FATAL_ERROR "Build step for googletest failed: ${result}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This defines
	# the gtest and gtest_main targets.
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
			${CMAKE_CURRENT_BINARY_DIR}/googletest-build
			EXCLUDE_FROM_ALL)

	# The gtest/gtest_main targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif()

    set(testFiles ../UbiForm/test_ComponentManifest.h ../UbiForm/test_SocketMessage.h
            ../UbiForm/SchemaRepresentation/test_EndpointSchema.h ../UbiForm/test_Component.h
            ../UbiForm/ResourceDiscovery/test_ResourceDiscoveryStore.h ../UbiForm/ResourceDiscovery/test_ComponentRepresentation.h
			../UbiForm/ResourceDiscovery/test_ResourceDiscoveryConnEndpoint.h ../UbiForm/SystemSchemas/test_SystemSchemas.h
			../tests/test_ReconfigurationIntegration.h ../UbiForm/Utilities/test_GetIPAddresses.h ../tests/test_Streaming.h)

    add_executable(run_tests ../tests/RunTests.cpp ${testFiles})
    target_link_libraries(run_tests gtest_main gmock_main UbiForm)
    file(COPY ../tests/TestManifests DESTINATION .)
endif(BUILD_TESTS)

# Create examples
if (BUILD_EXAMPLES)
    add_executable(pair_example ../examples/PairConnectionExample.cpp)
    target_link_libraries(pair_example UbiForm)

    add_executable(pubsub_example ../examples/PubSubExample.cpp)
    target_link_libraries(pubsub_example UbiForm)

    add_executable(reconfiguration_example ../examples/ReconfigurationExample.cpp)
    target_link_libraries(reconfiguration_example UbiForm)

    add_executable(resource_discovery_example ../examples/ResourceDiscoveryExample.cpp)
    target_link_libraries(resource_discovery_example UbiForm)

    # Copy over our base JSON files for our examples to use
    file(COPY ../examples/JsonFiles DESTINATION .)
endif(BUILD_EXAMPLES)