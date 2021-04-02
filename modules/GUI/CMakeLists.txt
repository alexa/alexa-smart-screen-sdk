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

add_custom_target(GUIClient ALL
        COMMAND "npm" "install"
        COMMAND "npm" "run" "build" "--"
            "--output-path" "${CMAKE_CURRENT_BINARY_DIR}"
            "--env" "DISABLE_WEBSOCKET_SSL=${DISABLE_WEBSOCKET_SSL_VALUE}"
            "--env" "USE_UWP_CLIENT=${USE_UWP_CLIENT_VALUE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/js")