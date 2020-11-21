cmake_minimum_required(VERSION 3.17)
project(UbiForm)

set(CMAKE_CXX_STANDARD 14)

# Add pthread library so NNG can build
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Set options whether we build tests and examples
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_EXAMPLES "Build examples programs" OFF)

# Note that both of these libraries need to be pre-installed at the moment, they don't come with

# Find library nng and set to variable NNG
find_library(NNG nng)
# Find package RapidJSON and it is included straight away
ADD_DEFINITIONS(-DRAPIDJSON_HAS_STDSTRING)
find_package(RapidJSON)


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

set(generalFunctions ../UbiForm/general_functions.h ../UbiForm/general_functions.cpp)
set(componentFiles ../UbiForm/ComponentManifest.h ../UbiForm/ComponentManifest.cpp ../UbiForm/Component.h ../UbiForm/Component.cpp
        ../UbiForm/SocketMessage.h ../UbiForm/SocketMessage.cpp)
set(endpointFiles ../UbiForm/endpoints/DataReceiverEndpoint.h ../UbiForm/endpoints/DataReceiverEndpoint.cpp
        ../UbiForm/endpoints/DataSenderEndpoint.h ../UbiForm/endpoints/DataSenderEndpoint.cpp
        ../UbiForm/endpoints/EndpointSchema.h ../UbiForm/endpoints/EndpointSchema.cpp
        ../UbiForm/endpoints/PairEndpoint.h ../UbiForm/endpoints/PairEndpoint.cpp)

# Create library
add_library(UbiForm ../UbiForm/library.cpp ../UbiForm/library.h ${componentFiles} ${generalFunctions} ${endpointFiles})
target_link_libraries(UbiForm gtest_main gmock_main)

# Create tests
if (BUILD_TESTS)
    set(testFiles ../UbiForm/test_ComponentManifest.h ../UbiForm/test_SocketMessage.h ../UbiForm/endpoints/test_EndpointSchema.h)
    add_executable(run_tests ../tests/RunTests.cpp ${testFiles})
    target_link_libraries(run_tests gtest_main gmock_main UbiForm ${NNG})
endif(BUILD_TESTS)

# Create examples
if (BUILD_EXAMPLES)
    add_executable(pair_example ../examples/PairConnectionExample.cpp)
    target_link_libraries(pair_example UbiForm ${NNG})

    # Copy over our base JSON files for our examples to use
    file(COPY ../examples/JsonFiles DESTINATION .)
endif(BUILD_EXAMPLES)