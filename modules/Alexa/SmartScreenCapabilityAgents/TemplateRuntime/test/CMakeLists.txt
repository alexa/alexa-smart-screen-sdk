cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
    "${TemplateRuntime_INCLUDE_DIR}"
    "${SmartScreenSDKInterfaces_SOURCE_DIR}/test"
    "${TemplateRuntime_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}"
        "${SmartScreenSDKInterfaces_SOURCE_DIR}/include"
        )

discover_unit_tests("${INCLUDE_PATH}" "SmartScreenTemplateRunTime")
