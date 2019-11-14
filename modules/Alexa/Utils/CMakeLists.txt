cmake_minimum_required(VERSION 3.1)

project(Utils LANGUAGES CXX)

include(../build/BuildDefaults.cmake)

add_library(Utils INTERFACE)

target_include_directories(Utils INTERFACE
    "${Utils_SOURCE_DIR}/include")
