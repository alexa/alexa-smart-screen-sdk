add_definitions("-DACSDK_LOG_MODULE=sampleApp")
set(SampleApp_SOURCES)
list(APPEND SampleApp_SOURCES
    AplCoreConnectionManager.cpp
    AplCoreEngineSDKLogBridge.cpp
    AplCoreGuiContentDownloadManager.cpp
    AplCoreGuiRenderer.cpp
    AplCoreMetrics.cpp
    AplCoreTextMeasurement.cpp
    ConnectionObserver.cpp
    ConsolePrinter.cpp
    GUILogBridge.cpp
    GUI/GUIClient.cpp
    GUI/GUIManager.cpp
    JsonUIManager.cpp
    KeywordObserver.cpp
    LocaleAssetsManager.cpp
    SampleApplication.cpp
    SampleEqualizerModeController.cpp
    main.cpp)

if (PORTAUDIO)
    list(APPEND SampleApp_SOURCES PortAudioMicrophoneWrapper.cpp)
endif()

IF (A4B)
    list(APPEND SampleApp_SOURCES RevokeAuthorizationObserver.cpp)
ENDIF()

if (PCC)
    list(APPEND SampleApp_SOURCES PhoneCaller.cpp)
endif()

IF (HAS_EXTERNAL_MEDIA_PLAYER_ADAPTERS)
    file(GLOB_RECURSE SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ExternalMediaAdapterRegistration/*.cpp)
    foreach(myfile ${SRC_FILE})
       list(APPEND SampleApp_SOURCES ${myfile})
    endforeach(myfile)
ENDIF()

add_executable(SampleApp ${SampleApp_SOURCES})
target_include_directories(SampleApp PUBLIC
    "${SampleApp_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}"
    "${AudioResources_SOURCE_DIR}/include"
    "${RegistrationManager_SOURCE_DIR}/include"
    "${ESP_SOURCE_DIR}/include"
    "${PORTAUDIO_INCLUDE_DIR}"
    "${Utils_SOURCE_DIR}/include"
    "${Communication_SOURCE_DIR/include}")

target_link_libraries(SampleApp
    "${ASDK_LDFLAGS}"
    Communication
    AlexaPresentation
    SmartScreenClient
    CapabilitiesDelegate
    CBLAuthDelegate
    DeviceSettings
    SQLiteStorage
    EqualizerImplementations
    TemplateRuntime
    Utils
    "${PORTAUDIO_LIB_PATH}")

if (PCC)
    target_link_libraries(SampleApp AVSCommon)
endif()

if (ANDROID)
    target_include_directories(SampleApp PUBLIC "${AndroidUtilities}/include")
    target_link_libraries(SampleApp AndroidUtilities)
    if (ANDROID_MEDIA_PLAYER)
        target_include_directories(SampleApp PUBLIC "${AndroidSLESMediaPlayer}/include")
        target_link_libraries(SampleApp AndroidSLESMediaPlayer)
    endif()
endif()

if (GSTREAMER_MEDIA_PLAYER)
    target_include_directories(SampleApp PUBLIC "${MediaPlayer_SOURCE_DIR}/include")
    target_link_libraries(SampleApp MediaPlayer)
endif()

if(KWD)
    target_link_libraries(SampleApp KeywordDetectorProvider)
endif()

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(FATAL_ERROR "WebSocketPP Path is required to enable websocket server")
endif()

if(NOT ASIO_INCLUDE_DIR)
    find_path(ASIO_INCLUDE_DIR NAMES "asio.hpp")

    if(ASIO_INCLUDE_DIR STREQUAL "ASIO_INCLUDE_DIR-NOTFOUND")
        message(FATAL_ERROR "WebSocket support requires asio.")
    endif()
endif()

target_compile_definitions(SampleApp PUBLIC ASIO_STANDALONE)
target_include_directories(SampleApp PUBLIC
        "${ASIO_INCLUDE_DIR}"
        "${WEBSOCKETPP_INCLUDE_DIR}")

# Currently only allow non SSL websocket with debug builds
if(DISABLE_WEBSOCKET_SSL)
    string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASE_CMAKE_BUILD_TYPE)
    if(NOT LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "debug")
        message(FATAL_ERROR "DISABLE_WEBSOCKET_SSL used in non-debug build.")
    endif()
else()
    find_package(OpenSSL REQUIRED)
    add_definitions(-DENABLE_WEBSOCKET_SSL)

    target_include_directories(SampleApp PUBLIC "${OPENSSL_INCLUDE_DIR}")
    target_link_libraries(SampleApp
            "${OPENSSL_SSL_LIBRARY}"
            "${OPENSSL_CRYPTO_LIBRARY}")
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(SampleApp
        "-rpath ${ASDK_LIBRARY_DIRS}"
        "-framework CoreAudio"
        "-framework AudioToolbox" 
        "-framework AudioUnit" 
        "-framework CoreServices" 
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(SampleApp
      rt m pthread asound atomic
      "-Wl,-rpath,${ASDK_LIBRARY_DIRS}")
endif()

if(NOT APLCORE_INCLUDE_DIR)
    message(FATAL_ERROR "APLCore Include Dir is required")
endif()

if(NOT APLCORE_LIB_DIR)
    message(FATAL_ERROR "APLCore Lib Dir is required")
endif()

if(NOT YOGA_INCLUDE_DIR)
    message(FATAL_ERROR "Yoga include dir is required")
endif()

if(NOT YOGA_LIB_DIR)
    message(FATAL_ERROR "Yoga lib dir is required")
endif()

if(APL_CORE)
    target_include_directories(SampleApp PUBLIC
            "${APLCORE_INCLUDE_DIR}"
            "${YOGA_INCLUDE_DIR}")

    target_link_libraries(SampleApp ${APLCORE_LIB_DIR}/libapl.a)
    target_link_libraries(SampleApp ${YOGA_LIB_DIR}/libyogacore.a)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error -Wno-reorder")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -Wno-reorder")

    add_definitions(-DAPL_CORE)
endif()
