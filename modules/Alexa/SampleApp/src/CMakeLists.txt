add_definitions("-DACSDK_LOG_MODULE=sampleApp")
list(APPEND SampleApp_SOURCES
    AplClientBridge.cpp
    ConnectionObserver.cpp
    CachingDownloadManager.cpp
    ConsolePrinter.cpp
    DownloadMonitor.cpp
    ExternalCapabilitiesBuilder.cpp
    GUILogBridge.cpp
    GUI/GUIClient.cpp
    GUI/GUIManager.cpp
    JsonUIManager.cpp
    KeywordObserver.cpp
    LocaleAssetsManager.cpp
    SampleApplication.cpp
    SampleApplicationComponent.cpp
    SampleEqualizerModeController.cpp
    SmartScreenCaptionPresenter.cpp
    SmartScreenCaptionStateManager.cpp
    TelemetrySink.cpp)

if (RTCSC)
    list(APPEND SampleApp_SOURCES Extensions/LiveView/AplLiveViewExtension.cpp)
endif()

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

add_library(SmartScreenSampleAppLib SHARED ${SampleApp_SOURCES})

target_include_directories(SmartScreenSampleAppLib PUBLIC
    "${SampleApp_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}"
    "${AudioResources_SOURCE_DIR}/include"
    "${RegistrationManager_SOURCE_DIR}/include"
    "${ESP_SOURCE_DIR}/include"
    "${PORTAUDIO_INCLUDE_DIR}"
    "${Utils_SOURCE_DIR}/include"
    "${Communication_SOURCE_DIR}/include"
    "${APLCLIENT_INCLUDE_DIRS}")

target_link_libraries(SmartScreenSampleAppLib
    "${ASDK_LDFLAGS}"
    APLClient
    Communication
    AlexaPresentation
    SmartScreenClient
    CapabilitiesDelegate
    CBLAuthDelegate
    DeviceSettings
    SQLiteStorage
    SSSDKCommon
    acsdkAudioInputStream
    acsdkEqualizerImplementations
    acsdkCrypto
    acsdkPkcs11
    TemplateRuntime
    Utils
    "${PORTAUDIO_LIB_PATH}")

if (RTCSC)
    target_link_libraries(SmartScreenSampleAppLib LiveViewControllerCapabilityAgent)
endif()

if (AUTH_MANAGER)
    target_link_libraries(SmartScreenSampleAppLib acsdkAuthorization)
endif()

if (PCC)
    target_link_libraries(SmartScreenSampleAppLib AVSCommon)
endif()

if (ANDROID)
    target_include_directories(SmartScreenSampleAppLib PUBLIC "${AndroidUtilities}/include")
    target_link_libraries(SmartScreenSampleAppLib AndroidUtilities)
    if (ANDROID_MEDIA_PLAYER)
        target_include_directories(SmartScreenSampleAppLib PUBLIC "${AndroidSLESMediaPlayer}/include")
        target_link_libraries(SmartScreenSampleAppLib AndroidSLESMediaPlayer)
    endif()
endif()

if (GSTREAMER_MEDIA_PLAYER)
    target_include_directories(SmartScreenSampleAppLib PUBLIC "${MediaPlayer_SOURCE_DIR}/include")
    target_link_libraries(SmartScreenSampleAppLib MediaPlayer)
endif()

if(KWD)
    target_link_libraries(SmartScreenSampleAppLib KeywordDetectorProvider)
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

target_compile_definitions(SmartScreenSampleAppLib PUBLIC ASIO_STANDALONE)
target_include_directories(SmartScreenSampleAppLib PUBLIC
        "${ASIO_INCLUDE_DIR}"
        "${WEBSOCKETPP_INCLUDE_DIR}")

if (CRYPTO_FOUND AND PKCS11)
    target_compile_definitions(SmartScreenSampleAppLib PRIVATE ENABLE_PKCS11)
endif()

# Currently only allow non SSL websocket with debug builds
if(DISABLE_WEBSOCKET_SSL)
    string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASE_CMAKE_BUILD_TYPE)
    if(NOT LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "debug")
        message(FATAL_ERROR "DISABLE_WEBSOCKET_SSL used in non-debug build.")
    endif()
else()
    find_package(OpenSSL REQUIRED)
    add_definitions(-DENABLE_WEBSOCKET_SSL)

    target_include_directories(SmartScreenSampleAppLib PUBLIC "${OPENSSL_INCLUDE_DIR}")
    target_link_libraries(SmartScreenSampleAppLib
            "${OPENSSL_SSL_LIBRARY}"
            "${OPENSSL_CRYPTO_LIBRARY}")
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(SmartScreenSampleAppLib
        "-rpath ${ASDK_LIBRARY_DIRS}"
        "-rpath ${APLCLIENT_LIBRARY_DIRS}"
        "-framework CoreAudio"
        "-framework AudioToolbox" 
        "-framework AudioUnit" 
        "-framework CoreServices" 
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(SmartScreenSampleAppLib
      rt m pthread asound atomic
      "-Wl,-rpath,${ASDK_LIBRARY_DIRS}"
      "-Wl,-rpath,${APLCLIENT_LIBRARY_DIRS}")
endif()


add_executable(SampleApp main.cpp)
target_link_libraries(SampleApp SmartScreenSampleAppLib)

# install target
asdk_install_targets(SmartScreenSampleAppLib TRUE)
