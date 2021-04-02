if (((PORTAUDIO OR AUDIO_INJECTION) AND GSTREAMER_MEDIA_PLAYER) OR (ANDROID_MEDIA_PLAYER AND ANDROID_MICROPHONE))
    cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
    project(SampleApp LANGUAGES CXX)

    include(../build/BuildDefaults.cmake)

    if (PORTAUDIO)
        add_definitions("-DPORTAUDIO")
    endif()

    if (GSTREAMER_MEDIA_PLAYER)
        add_definitions("-DGSTREAMER_MEDIA_PLAYER")
    endif()

    if (AUDIO_INJECTION)
        add_definitions("-DAUDIO_INJECTION")
    endif()

    string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASE_CMAKE_BUILD_TYPE)

    # Enable validation of configurations files only in debug mode
    if (LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "debug")
        add_definitions("-DENABLE_CONFIG_VALIDATION")
        file(COPY "${SampleApp_SOURCE_DIR}/schemas/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/schemas")
    endif()

    add_subdirectory("src")
    add_subdirectory("test")
else()
    message("To build the sample app, please enable microphone and media player modules.")
endif()
