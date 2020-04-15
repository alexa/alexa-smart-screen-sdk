cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(AlexaModule LANGUAGES CXX)

find_package(PkgConfig)
pkg_check_modules(ASDK REQUIRED AlexaClientSDK=1.19.0)

include(build/BuildDefaults.cmake)
include(tools/Testing.cmake)

# Set variables for target install and .pc pkg-config file
include(build/cmake/PrepareInstall.cmake)

# Alexa Client SDK targets.
add_subdirectory("ThirdParty")
add_subdirectory("SmartScreenCapabilityAgents")
add_subdirectory("SmartScreenSDKInterfaces")
add_subdirectory("ApplicationUtilities")
add_subdirectory("SampleApp")
add_subdirectory("doc")
add_subdirectory("Utils")

# Create .pc pkg-config file
include(build/cmake/GeneratePkgConfig.cmake)
