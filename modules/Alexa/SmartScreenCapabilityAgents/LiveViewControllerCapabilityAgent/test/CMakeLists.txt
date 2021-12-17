cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(INCLUDE_PATH
        "${LiveViewControllerCapabilityAgent_INCLUDE_DIR}"
        "${LiveViewControllerCapabilityAgent}/include"
        "${ASDK_INCLUDE_DIRS}"
        )

discover_unit_tests("${INCLUDE_PATH}" "LiveViewControllerCapabilityAgent")