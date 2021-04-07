cmake_minimum_required(VERSION 3.17)
project(UbiForm)

set(CMAKE_CXX_STANDARD 14)

# Add pthread library so NNG can build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Up the levels of optimisation when releasing
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Set options whether we build tests and examples
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_EXAMPLES "Build examples programs" OFF)
option(BUILD_SOUND_EXAMPLES "Build sound based examples" OFF)
option(BUILD_EVALUATION "Build evaluation programs" OFF)



# Setup nng library
include_directories(nng-master/include)
add_subdirectory(nng-master)

# Setup rapidjson
ADD_DEFINITIONS(-DRAPIDJSON_HAS_STDSTRING)
set(rapidjson_dir ${CMAKE_CURRENT_SOURCE_DIR}/../libs/rapidjson)
include_directories(${rapidjson_dir}/include)

# Control the view of the output
option(VIEW_STD_OUTPUT "Standard output used" ON)
if(VIEW_STD_OUTPUT)
	add_compile_definitions(VIEW_STD_OUTPUT=true)
else()
	add_compile_definitions(VIEW_STD_OUTPUT=false)
endif()

# Control the location of systems scheams pointer
file(COPY ../UbiForm/SystemSchemas DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(SYSTEM_SCHEMAS_LOC="${CMAKE_CURRENT_BINARY_DIR}/SystemSchemas")

# File setup
set(endpointFiles ../include/UbiForm/Endpoints/DataReceiverEndpoint.h ../UbiForm/Endpoints/DataReceiverEndpoint.cpp
        ../include/UbiForm/Endpoints/DataSenderEndpoint.h ../UbiForm/Endpoints/DataSenderEndpoint.cpp
        ../include/UbiForm/Endpoints/PairEndpoint.h ../UbiForm/Endpoints/PairEndpoint.cpp
        ../UbiForm/Endpoints/SubscriberEndpoint.cpp ../include/UbiForm/Endpoints/SubscriberEndpoint.h
        ../UbiForm/Endpoints/PublisherEndpoint.cpp ../include/UbiForm/Endpoints/PublisherEndpoint.h
		../UbiForm/Endpoints/RequestEndpoint.cpp ../include/UbiForm/Endpoints/RequestEndpoint.h
		../UbiForm/Endpoints/ReplyEndpoint.cpp ../include/UbiForm/Endpoints/ReplyEndpoint.h
		../UbiForm/Endpoints/Endpoint.cpp ../include/UbiForm/Endpoints/Endpoint.h)

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
		../UbiForm/Utilities/GetIPAddress.cpp ../UbiForm/Utilities/GetIPAddress.h
		../UbiForm/Utilities/base64.h ../UbiForm/Utilities/base64.cpp
		../include/UbiForm/Utilities/ExceptionClasses.h ../UbiForm/Utilities/ExceptionClasses.cpp)

set(componentFiles ../include/UbiForm/ComponentManifest.h ../UbiForm/ComponentManifest.cpp
		../include/UbiForm/Component.h ../UbiForm/Component.cpp
        ../include/UbiForm/EndpointMessage.h ../UbiForm/EndpointMessage.cpp)


# Create library
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

    set(testFiles ../UbiForm/test_ComponentManifest.h ../UbiForm/test_EndpointMessage.h
            ../UbiForm/SchemaRepresentation/test_EndpointSchema.h ../UbiForm/test_Component.h
            ../UbiForm/ResourceDiscovery/test_ResourceDiscoveryStore.h ../UbiForm/ResourceDiscovery/test_ComponentRepresentation.h
			../UbiForm/ResourceDiscovery/test_ResourceDiscoveryConnEndpoint.h ../UbiForm/SystemSchemas/test_SystemSchemas.h
			../tests/test_ReconfigurationIntegration.h ../UbiForm/Utilities/test_GetIPAddresses.h ../tests/test_Streaming.h ../tests/test_EndpointCreation.h)

    add_executable(run_tests ../tests/RunTests.cpp ${testFiles})
    target_link_libraries(run_tests gtest_main gmock_main UbiForm)
	set_target_properties(run_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testing)
    file(COPY ../tests/TestManifests DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/testing)
	file(COPY ../tests/TestFiles DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/testing)
endif(BUILD_TESTS)

# Create examples
if (BUILD_EXAMPLES)
    add_executable(pair_example ../examples/PairConnectionExample.cpp)
	set_target_properties(pair_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
    target_link_libraries(pair_example UbiForm)

    add_executable(pubsub_example ../examples/PubSubExample.cpp)
	set_target_properties(pubsub_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
    target_link_libraries(pubsub_example UbiForm)

    add_executable(reconfiguration_example ../examples/ReconfigurationExample.cpp)
	set_target_properties(reconfiguration_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
	target_link_libraries(reconfiguration_example UbiForm)

    add_executable(resource_discovery_example ../examples/ResourceDiscoveryExample.cpp)
	set_target_properties(resource_discovery_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
	target_link_libraries(resource_discovery_example UbiForm)

	add_executable(simple_stream_example ../examples/SimpleStreamingExample.cpp)
	set_target_properties(simple_stream_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
	target_link_libraries(simple_stream_example UbiForm)

    # Copy over our base JSON files for our examples to use
    file(COPY ../examples/JsonFiles DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/examples)
endif(BUILD_EXAMPLES)

if(BUILD_SOUND_EXAMPLES)
	add_executable(music_stream ../examples/MusicPlayerExample.cpp)
	set_target_properties(music_stream PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
	target_link_libraries(music_stream UbiForm sfml-audio sfml-system)
endif(BUILD_SOUND_EXAMPLES)

if(BUILD_EVALUATION)
	file(COPY ../evaluation/EvaluationFiles DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/evaluation)


	add_executable(UbiForm_stream_speed_test ../evaluation/streamingSpeedEvaluation/streamingSpeedUbiForm.cpp)
	set_target_properties(UbiForm_stream_speed_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(UbiForm_stream_speed_test UbiForm)

	add_executable(nng_message_speed_test ../evaluation/streamingSpeedEvaluation/messageSpeedNNG.cpp)
	set_target_properties(nng_message_speed_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(nng_message_speed_test nng)

	add_executable(nng_stream_speed_test ../evaluation/streamingSpeedEvaluation/streamingSpeedNNG.cpp)
	set_target_properties(nng_stream_speed_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(nng_stream_speed_test nng)

	add_executable(pub_sub_speed_test ../evaluation/pubSubSpeed.cpp)
	set_target_properties(pub_sub_speed_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(pub_sub_speed_test UbiForm)
	configure_file(../evaluation/runPubSubEval.sh ${CMAKE_CURRENT_BINARY_DIR}/run_pub_sub_eval.sh COPYONLY)

	add_executable(brokered_speed_test_broker ../evaluation/brokeredPubSubEvaluation/Broker.cpp)
	set_target_properties(brokered_speed_test_broker PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(brokered_speed_test_broker UbiForm)
	add_executable(brokered_speed_test_pubsub ../evaluation/brokeredPubSubEvaluation/PublisherSubscriber.cpp)
	set_target_properties(brokered_speed_test_pubsub PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(brokered_speed_test_pubsub UbiForm)
	configure_file(../evaluation/brokeredPubSubEvaluation/run_brokered_pub_sub.sh ${CMAKE_CURRENT_BINARY_DIR}/run_brokered_pub_sub.sh COPYONLY)

	add_executable(section_speed_test ../evaluation/sectionSpeeds.cpp)
	set_target_properties(section_speed_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(section_speed_test UbiForm)

	add_executable(scalability_streaming_test ../evaluation/scalabilityStreaming.cpp)
	set_target_properties(scalability_streaming_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(scalability_streaming_test UbiForm)

	add_executable(resource_discovery_scalability_query ../evaluation/scalabilityResourceDiscovery/resourceDiscoveryQuery.cpp)
	set_target_properties(resource_discovery_scalability_query PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(resource_discovery_scalability_query UbiForm)
	add_executable(resource_discovery_scalability_register ../evaluation/scalabilityResourceDiscovery/resourceDiscoveryRegister.cpp)
	set_target_properties(resource_discovery_scalability_register PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/evaluation)
	target_link_libraries(resource_discovery_scalability_register UbiForm)
endif(BUILD_EVALUATION)