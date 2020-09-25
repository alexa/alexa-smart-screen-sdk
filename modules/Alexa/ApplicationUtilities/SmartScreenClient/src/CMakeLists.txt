cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

add_definitions("-DACSDK_LOG_MODULE=smartScreenClient")
add_library(SmartScreenClient SHARED
    SmartScreenClient.cpp
    EqualizerRuntimeSetup.cpp
    DeviceSettingsManagerBuilder.cpp)

target_include_directories(SmartScreenClient PUBLIC
    "${SmartScreenClient_SOURCE_DIR}/include"
    "${AlexaPresentation_SOURCE_DIR}/include"
    "${SmartScreenTemplateRunTime_SOURCE_DIR}/include"
    "${VisualCharacteristics_SOURCE_DIR}/include"
    "${ASDK_INCLUDE_DIRS}"
    "${SQLITE_INCLUDE_DIRS}")

if(BLUETOOTH_BLUEZ)
    target_link_libraries(SmartScreenClient
        BluetoothImplementationsBlueZ)
endif()

target_link_libraries(SmartScreenClient SmartScreenTemplateRunTime "${ASDK_LDFLAGS}" AlexaPresentation VisualCharacteristics)

if (COMMS)
    target_link_libraries(SmartScreenClient CallManager)
endif()

if (CAPTIONS)
    if (NOT MSVC)
        target_link_libraries(SmartScreenClient CaptionsLib)
    else()
        target_link_libraries(SmartScreenClient "${ASDK_LIB_DIR}/CaptionsLib.lib")
    endif()
endif()

if (METRICS_EXTENSION)
    target_include_directories(SmartScreenClient PUBLIC "${MetricsExtension_SOURCE_DIR}/include")
    target_link_libraries(SmartScreenClient "MetricsExtension")
endif()

if (PCC)
    target_link_libraries(SmartScreenClient PhoneCallController)
endif()

if (MCC)
    target_link_libraries(SmartScreenClient MeetingClientController)
endif()

if (MRM AND MRM_STANDALONE_APP)
    target_link_libraries(SmartScreenClient MRM MRMHandlerProxy)
elseif (MRM)
    target_link_libraries(SmartScreenClient MRM MRMHandler)
endif()

if (METRICS)
    target_link_libraries(SmartScreenClient SampleMetricSink MetricRecorder)
endif()

if (OPUS)
    target_link_libraries(SmartScreenClient OpusEncoderContext)
endif()

# install target
asdk_install()
