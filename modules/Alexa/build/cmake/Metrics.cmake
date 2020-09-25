#
# Setup the Metrics compiler options.
#
# To build with Metrics support, include the following option on the cmake command line.
#     cmake <path-to-source> -DMETRICS=ON [-DMETRICS_EXTENSION={ON|OFF}]
#
option(METRICS "Enable Metrics upload." OFF)
option(METRICS_EXTENSION "Enable Metrics Extension support." OFF)
option(USE_APL_TELEMETRY "Use APL Telemetry for document metrics" OFF)

if(METRICS)
    message("Creating ${PROJECT_NAME} with Metrics upload")
    add_definitions(-DACSDK_ENABLE_METRICS_RECORDING)

    if(METRICS_EXTENSION)
        message("Creating ${PROJECT_NAME} with Metrics Extension")
        add_definitions(-DMETRICS_EXTENSION)
    endif()
endif()

if(METRICS AND USE_APL_TELEMETRY)
    message("Creating ${PROJECT_NAME} with APL Telemetry support")
    add_definitions(-DENABLE_APL_TELEMETRY)
endif()
