cmake_minimum_required(VERSION 3.21)
project(ToastyWindowAnalyzer VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK is not set. Point it at your local Geode SDK.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_sources(${PROJECT_NAME} PRIVATE
    src/Main.cpp
    src/TTRL.cpp
    src/Timeline.cpp
    src/WindowAnalyzer.cpp
    src/WindowOverlay.cpp
    src/GameplayProbe.cpp
)
