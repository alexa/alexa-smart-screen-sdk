#
# Setup the Keyword Detector type and compiler options.
#
# To build with a Keyword Detector, run the following command with a keyword detector type of AMAZON_KEY_WORD_DETECTOR,
# AMAZONLITE_KEY_WORD_DETECTOR, or SENSORY_KEY_WORD_DETECTOR:
#     cmake <path-to-source> 
#       -DAMAZON_KEY_WORD_DETECTOR=ON 
#           -DAMAZON_KEY_WORD_DETECTOR_LIB_PATH=<path-to-amazon-lib> 
#           -DAMAZON_KEY_WORD_DETECTOR_INCLUDE_DIR=<path-to-amazon-include-dir>
#       -DAMAZONLITE_KEY_WORD_DETECTOR=ON 
#       -DSENSORY_KEY_WORD_DETECTOR=ON 
#           -DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=<path-to-sensory-lib>
#           -DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=<path-to-sensory-include-dir>
#

option(AMAZON_KEY_WORD_DETECTOR "Enable Amazon keyword detector." OFF)
option(AMAZONLITE_KEY_WORD_DETECTOR "Enable AmazonLite keyword detector." OFF)
option(SENSORY_KEY_WORD_DETECTOR "Enable Sensory keyword detector." OFF)

if(NOT AMAZON_KEY_WORD_DETECTOR AND NOT AMAZONLITE_KEY_WORD_DETECTOR AND NOT SENSORY_KEY_WORD_DETECTOR)
    message("No keyword detector type specified, skipping build of keyword detector.")
    return()
endif()

if(AMAZON_KEY_WORD_DETECTOR)
    message("Creating ${PROJECT_NAME} with keyword detector type: Amazon")
    if(NOT AMAZON_KEY_WORD_DETECTOR_LIB_PATH)
        message(FATAL_ERROR "Must pass library path of Amazon KeywordDetector!")
    endif()
    if(NOT AMAZON_KEY_WORD_DETECTOR_INCLUDE_DIR)
        message(FATAL_ERROR "Must pass include dir path of Amazon KeywordDetector!")
    endif()
    add_definitions(-DKWD)
    add_definitions(-DKWD_AMAZON)
    set(KWD ON)
endif()

if(AMAZONLITE_KEY_WORD_DETECTOR)
    message("Creating ${PROJECT_NAME} with keyword detector type: AmazonLite")
    add_definitions(-DKWD)
    set(KWD ON)
endif()

if(SENSORY_KEY_WORD_DETECTOR)
    message("Creating ${PROJECT_NAME} with keyword detector type: Sensory")
    if(NOT SENSORY_KEY_WORD_DETECTOR_LIB_PATH)
        message(FATAL_ERROR "Must pass library path of Sensory KeywordDetector!")
    endif()
    if(NOT SENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR)
        message(FATAL_ERROR "Must pass include dir path of Sensory KeywordDetector!")
    endif()
    add_definitions(-DKWD)
    add_definitions(-DKWD_SENSORY)
    set(KWD ON)
endif()
