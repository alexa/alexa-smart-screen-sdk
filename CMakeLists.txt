cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(AlexaSmartScreenSDK VERSION 2.4 LANGUAGES CXX)
set(PROJECT_BRIEF "A cross-platform, modular SDK for multi modal interaction with the Alexa Voice Service")
configure_file (
        "${PROJECT_SOURCE_DIR}/modules/Alexa/Utils/include/Utils/SmartScreenSDKVersion.h.in"
        "${PROJECT_SOURCE_DIR}/modules/Alexa/Utils/include/Utils/SmartScreenSDKVersion.h"
)
add_subdirectory("modules")
