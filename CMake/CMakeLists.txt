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

set(generalFunctions ../UbiForm/general_functions.h)
set(componentFiles ../UbiForm/ComponentManifest.h ../UbiForm/ComponentManifest.cpp ../UbiForm/Component.h ../UbiForm/Component.cpp)

# Create library
add_library(UbiForm ../UbiForm/library.cpp ../UbiForm/library.h ${componentFiles} ${generalFunctions})
target_link_libraries(UbiForm gtest_main gmock_main)

# Create tests
if (BUILD_TESTS)
    add_executable(component_unittest ../UbiForm/test_ComponentManifest.cpp)
    target_link_libraries(component_unittest gtest_main gmock_main UbiForm)
endif(BUILD_TESTS)

# Create examples
if (BUILD_EXAMPLES)
    add_executable(pair_example ../examples/PairConnectionExample.cpp)
    target_link_libraries(pair_example UbiForm ${NNG})
endif(BUILD_EXAMPLES)