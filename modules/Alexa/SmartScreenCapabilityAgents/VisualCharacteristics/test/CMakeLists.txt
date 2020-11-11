cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
    "${VisualCharacteristics_INCLUDE_DIR}"
    "${VisualCharacteristics_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}")

discover_unit_tests("${INCLUDE_PATH}" "VisualCharacteristics")
