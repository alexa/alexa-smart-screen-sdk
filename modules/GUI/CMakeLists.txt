cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GUIClient)

if(DISABLE_WEBSOCKET_SSL)
    set(DISABLE_WEBSOCKET_SSL_VALUE "true")
else()
    set(DISABLE_WEBSOCKET_SSL_VALUE "false")
endif()

if(USE_UWP_CLIENT)
    set(USE_UWP_CLIENT_VALUE "true")
else()
    set(USE_UWP_CLIENT_VALUE "false")
endif()

if(RTCSC)
    set(RTCSC_ENABLED_VALUE "true")
    set(RTCSC_JS_PATH_VALUE "${RTCSC_JS_PATH}")
else()
    set(RTCSC_ENABLED_VALUE "false")
    set(RTCSC_JS_PATH_VALUE "${CMAKE_CURRENT_SOURCE_DIR}/js/src/lib/liveView/RTCSC_Adapter_Stub")
endif()

add_custom_target(GUIClient ALL
        COMMAND "npm" "install"
        COMMAND "npm" "install" "${APL_CLIENT_JS_PATH}"
        COMMAND "npm" "install" "${RTCSC_JS_PATH_VALUE}"
        COMMAND "npm" "run" "build" "--"
            "--output-path" "${CMAKE_CURRENT_BINARY_DIR}"
            "--env" "DISABLE_WEBSOCKET_SSL=${DISABLE_WEBSOCKET_SSL_VALUE}"
            "--env" "USE_UWP_CLIENT=${USE_UWP_CLIENT_VALUE}"
            "--env" "RTCSC_ENABLED=${RTCSC_ENABLED_VALUE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/js")
