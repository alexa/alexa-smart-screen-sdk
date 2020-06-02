cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(modules LANGUAGES CXX)

add_subdirectory("Alexa")
if (NOT MSVC)
	# For UWP, build the GUIClient on UWP branch on a different platform
	add_subdirectory("GUI")
endif()