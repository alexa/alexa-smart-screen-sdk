add_definitions("-DACSDK_LOG_MODULE=templateRuntime")

add_library(SmartScreenTemplateRunTime SHARED
    "${CMAKE_CURRENT_LIST_DIR}/TemplateRuntime.cpp")

target_include_directories(SmartScreenTemplateRunTime
    PUBLIC "${SmartScreenTemplateRunTime_SOURCE_DIR}/include"
        "${SmartScreenSDKInterfaces_SOURCE_DIR}/include"
        "${ASDK_INCLUDE_DIRS}")

target_link_libraries(SmartScreenTemplateRunTime "${ASDK_LDFLAGS}" SmartScreenSDKInterfaces)


# install target
asdk_install()