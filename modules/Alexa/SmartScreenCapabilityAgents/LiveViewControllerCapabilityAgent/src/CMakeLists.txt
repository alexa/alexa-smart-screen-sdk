add_definitions("-DACSDK_LOG_MODULE=liveViewControllerCapabilityAgent")

add_library(LiveViewControllerCapabilityAgent SHARED
        "LiveViewControllerCapabilityAgent.cpp")

target_include_directories(LiveViewControllerCapabilityAgent
        PUBLIC
            "../include"
            "${ASDK_INCLUDE_DIRS}"
            ${RTCSC_INCLUDE_DIR})

target_link_libraries(LiveViewControllerCapabilityAgent
        "${ASDK_LDFLAGS}"
        SmartScreenSDKInterfaces
        ${RTCSC_LIB_PATH})

# install target
asdk_install()