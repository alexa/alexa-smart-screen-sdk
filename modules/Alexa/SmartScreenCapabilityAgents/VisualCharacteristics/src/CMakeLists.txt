add_definitions("-DACSDK_LOG_MODULE=visualCharacteristics")

add_library(VisualCharacteristics SHARED
    "${CMAKE_CURRENT_LIST_DIR}/VisualCharacteristics.cpp")

target_include_directories(VisualCharacteristics
    PUBLIC "${VisualCharacteristics_SOURCE_DIR}/include"
        "${SmartScreenSDKInterfaces_SOURCE_DIR}/include"
        "${ASDK_INCLUDE_DIRS}"
        "${RAPIDJSON_INCLUDE_DIR}")

target_link_libraries(VisualCharacteristics "${ASDK_LDFLAGS}" SmartScreenSDKInterfaces)

# install target
asdk_install()