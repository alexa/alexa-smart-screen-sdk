cmake_minimum_required(VERSION 3.0)
project(SmartScreenSDKInterfaces LANGUAGES CXX)

add_subdirectory("test")

add_library(SmartScreenSDKInterfaces INTERFACE)

target_include_directories(SmartScreenSDKInterfaces INTERFACE
    "${SmartScreenSDKInterfaces_SOURCE_DIR}/include"
    "${SmartScreenSDKInterfaces_SOURCE_DIR}/test")

asdk_install()
