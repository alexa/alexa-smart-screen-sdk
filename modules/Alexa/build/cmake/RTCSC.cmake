#
# Setup the RTCSC compiler options.
#
# To build with RTCSC support, include the following option on the cmake command line.
#     cmake <path-to-source>
#       -DRTCSC=ON
#           -DRTCSC_LIB_PATH=<path-to-rtcsc-lib>
#           -DRTCSC_INCLUDE_DIR=<path-to-rtcsc-include-dir>
#

option(RTCSC "Enable RTCSC." OFF)

if(RTCSC)
    if(NOT RTCSC_LIB_PATH)
        message(FATAL_ERROR "Must pass path to the external library to enable RTCSC support.")
    endif()
    if(NOT RTCSC_INCLUDE_DIR)
        message(FATAL_ERROR "Must pass include directory path to enable RTCSC support.")
    endif()
    message("Creating ${PROJECT_NAME} with RTCSC")
    add_definitions(-DENABLE_RTCSC)
endif()