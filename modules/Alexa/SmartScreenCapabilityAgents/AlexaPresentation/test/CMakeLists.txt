cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
    "${AlexaPresentation_INCLUDE_DIR}"
    "${AlexaPresentation_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}"
    "${RAPIDJSON_INCLUDE_DIR}")

discover_unit_tests("${INCLUDE_PATH}" "AlexaPresentation")
