cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

add_definitions("-DACSDK_LOG_MODULE=SSSDKCommon")
add_library(SSSDKCommon SHARED
    AudioFileUtil.cpp
    NullMediaSpeaker.cpp
    NullMicrophone.cpp
    TestMediaPlayer.cpp)

target_include_directories(SSSDKCommon
    PUBLIC "${SSSDKCommon_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}")

target_link_libraries(SSSDKCommon "${ASDK_LDFLAGS}")

asdk_install()
