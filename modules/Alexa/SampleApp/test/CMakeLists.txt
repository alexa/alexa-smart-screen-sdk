cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
        "${SampleApp_INCLUDE_DIR}"
        "${ASDK_INCLUDE_DIRS}"
        "${SmartScreenSDKInterfaces_SOURCE_DIR}/include")

discover_unit_tests("${INCLUDE_PATH}" "SmartScreenSampleAppLib")
