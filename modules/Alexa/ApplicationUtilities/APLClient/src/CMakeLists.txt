cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

add_library(APLClient SHARED
    AplClientBinding.cpp
    AplCoreConnectionManager.cpp
    AplCoreEngineLogBridge.cpp
    AplCoreGuiRenderer.cpp
    AplCoreMetrics.cpp
    AplCoreTextMeasurement.cpp
    )

target_include_directories(APLClient PUBLIC
    "${ASDK_INCLUDE_DIRS}"
    "${APLClient_SOURCE_DIR}/include")


if(NOT APLCORE_INCLUDE_DIR)
    message(FATAL_ERROR "APLCore Include Dir is required")
endif()

if(NOT APLCORE_LIB_DIR)
    message(FATAL_ERROR "APLCore Lib Dir is required")
endif()

if(APL_CORE)
    if (NOT MSVC)
        target_include_directories(APLClient PUBLIC
                "${APLCORE_INCLUDE_DIR}"
                "${YOGA_INCLUDE_DIR}")

        target_link_libraries(APLClient ${APLCORE_LIB_DIR}/libapl.a)
        target_link_libraries(APLClient ${YOGA_LIB_DIR}/libyogacore.a)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error -Wno-reorder")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -Wno-reorder")
    elseif(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /GS /TP /W3 /Zc:wchar_t /DAPL_CORE_UWP /Zc:inline /Zc:twoPhase- /fp:precise /DWIN32_LEAN_AND_MEAN /DNOGDI /DWIN32 /D_WINDOWS")
        target_include_directories(APLClient PUBLIC
                "${APLCORE_INCLUDE_DIR}"
                "${YOGA_INCLUDE_DIR}")

        target_link_libraries(APLClient ${APLCORE_LIB_DIR}/apl.lib)
        target_link_libraries(APLClient ${YOGA_LIB_DIR}/yogacore.lib)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    add_definitions(-DAPL_CORE)
endif()

# install target
asdk_install()

