cmake_minimum_required(VERSION 3.0)
project(CapabilityAgents LANGUAGES CXX)

include(../build/BuildDefaults.cmake)

add_subdirectory("AlexaPresentation")
add_subdirectory("TemplateRuntime")
add_subdirectory("VisualCharacteristics")

if (RTCSC)
    add_subdirectory("LiveViewControllerCapabilityAgent")
endif()