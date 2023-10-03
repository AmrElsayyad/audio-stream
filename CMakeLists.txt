cmake_minimum_required(VERSION 3.27)

# The project name
project(audio_stream)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include modules
include(ExternalProject)
include(FetchContent)

# add Boost as an external project
find_package(Boost QUIET)
if(NOT Boost_FOUND)
    ExternalProject_Add(boost
        URL https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.tar.bz2
        PREFIX ${CMAKE_BINARY_DIR}/boost
        CONFIGURE_COMMAND cd ../boost && ./bootstrap.sh
        BUILD_COMMAND cd ../boost && sudo ./b2 install && sudo ldconfig
        INSTALL_COMMAND ""
    )
endif()

# add PortAudio as an external project
ExternalProject_Add(PortAudio
    URL https://github.com/PortAudio/portaudio/archive/refs/tags/v19.7.0.tar.gz
    PREFIX ${CMAKE_BINARY_DIR}/PortAudio
    CONFIGURE_COMMAND cd ../PortAudio && sudo apt update && sudo apt install libasound2-dev && ./configure --without-jack
    BUILD_COMMAND cd ../PortAudio && make
    INSTALL_COMMAND ""
)
set(PortAudio_INCLUDE_DIR ${CMAKE_BINARY_DIR}/PortAudio/src/PortAudio/include)
set(PortAudio_LIBRARY_DIR ${CMAKE_BINARY_DIR}/PortAudio/src/PortAudio/lib/.libs)

# Fetch GTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(googletest)

# include directories
include_directories(${PortAudio_INCLUDE_DIR})

# find packages
find_package(Boost COMPONENTS log filesystem REQUIRED)

# source files
set(source_dir "${PROJECT_SOURCE_DIR}/src/")
file(GLOB source_files "${source_dir}/*.cpp")

# add the executable
add_executable(audio_stream ${source_files})

# link libraries to the executable
target_link_libraries(audio_stream pthread Boost::log ${PortAudio_LIBRARY_DIR}/libportaudio.a asound)

# test files
file(GLOB test_files "${PROJECT_SOURCE_DIR}/test/*.cpp")

# add the test executable
add_executable(run_tests ${test_files})

# link libraries to the test executable
target_link_libraries(run_tests gtest_main Boost::log)
