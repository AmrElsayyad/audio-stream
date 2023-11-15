cmake_minimum_required(VERSION 3.21)

project(AudioStream)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VCPKG_ROOT ${CMAKE_SOURCE_DIR}/vcpkg)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

file(GLOB VCPKG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vcpkg_installed/*/include)
file(GLOB VCPKG_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/vcpkg_installed/*/lib)
file(GLOB VCPKG_SHARE_DIR ${CMAKE_SOURCE_DIR}/vcpkg_installed/*/share/*)

include_directories(./ ${VCPKG_INCLUDE_DIR})
link_directories(${VCPKG_LIBRARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${VCPKG_SHARE_DIR})

set(Boost_INCLUDE_DIR ${VCPKG_INCLUDE_DIR})

find_package(Boost REQUIRED COMPONENTS log program_options)
find_package(portaudio REQUIRED)
find_package(GTest REQUIRED)

enable_testing()

add_subdirectory(bluetooth-serial-port)
add_subdirectory(src)
add_subdirectory(test)
