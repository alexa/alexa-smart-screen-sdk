add_definitions("-DACSDK_LOG_MODULE=alexaPresentation")

add_library(AlexaPresentation SHARED
    "${CMAKE_CURRENT_LIST_DIR}/AlexaPresentation.cpp")

target_include_directories(AlexaPresentation
    PUBLIC "${AlexaPresentation_SOURCE_DIR}/include"
        "${SmartScreenSDKInterfaces_SOURCE_DIR}/include"
        "${ASDK_INCLUDE_DIRS}"
        "${RAPIDJSON_INCLUDE_DIR}")

target_link_libraries(AlexaPresentation "${ASDK_LDFLAGS}" SmartScreenSDKInterfaces)

# install target
asdk_install()