set(APLClientSandbox_SOURCES)
list(APPEND APLClientSandbox_SOURCES
    AplClientBridge.cpp
    Executor.cpp
    GUIManager.cpp
    Logger.cpp
    WebSocketSDKLogger.cpp
    WebSocketServer.cpp
    main.cpp)

add_executable(APLClientSandbox ${APLClientSandbox_SOURCES})
target_include_directories(APLClientSandbox PUBLIC
    "${APLClientSandbox_SOURCE_DIR}/include"
    "${APLClient_SOURCE_DIR/include}")

target_link_libraries(APLClientSandbox
    APLClient
    "${PORTAUDIO_LIB_PATH}")

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(FATAL_ERROR "WebSocketPP Path is required to enable websocket server")
endif()

if(NOT ASIO_INCLUDE_DIR)
    find_path(ASIO_INCLUDE_DIR NAMES "asio.hpp")

    if(ASIO_INCLUDE_DIR STREQUAL "ASIO_INCLUDE_DIR-NOTFOUND")
        message(FATAL_ERROR "WebSocket support requires asio.")
    endif()
endif()

target_compile_definitions(APLClientSandbox PUBLIC ASIO_STANDALONE)
target_include_directories(APLClientSandbox PUBLIC
    "${ASIO_INCLUDE_DIR}"
    "${WEBSOCKETPP_INCLUDE_DIR}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(APLClientSandbox
        "-framework CoreAudio"
        "-framework AudioToolbox" 
        "-framework AudioUnit" 
        "-framework CoreServices" 
        "-framework Carbon")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(APLClientSandbox
      rt m pthread asound atomic)
endif()
