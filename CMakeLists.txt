cmake_minimum_required(VERSION 3.21)
project(CBot VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED GEODE_SDK)
    message(FATAL_ERROR "GEODE_SDK is not set")
endif()

add_subdirectory(${GEODE_SDK} geode-sdk)
setup_geode_mod()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_geode_mod(cbot ${SOURCES})
