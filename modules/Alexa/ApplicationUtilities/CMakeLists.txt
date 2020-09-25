cmake_minimum_required(VERSION 3.1)
project(ApplicationUtilities LANGUAGES CXX)

include(../build/BuildDefaults.cmake)

if (METRICS_EXTENSION)
    add_subdirectory("Metrics")
endif ()

if (NOT MSVC)
    add_subdirectory("Communication")
endif ()
add_subdirectory("SmartScreenClient")

