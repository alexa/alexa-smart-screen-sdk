cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
    "${APLClient_INCLUDE_DIR}"
    "${APLClient_SOURCE_DIR}/include"
    "${APLCORE_RAPIDJSON_INCLUDE_DIR}")

discover_unit_tests("${INCLUDE_PATH}" "APLClient")
