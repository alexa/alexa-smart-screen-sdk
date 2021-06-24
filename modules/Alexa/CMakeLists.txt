cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(AlexaSmartScreenSDK LANGUAGES CXX)

if (NOT MSVC)
	find_package(PkgConfig)
	pkg_check_modules(ASDK REQUIRED AlexaClientSDK=1.24.0)
	set(ASDK_LIBRARY_DIRS "${CMAKE_PREFIX_PATH}/lib")
	# Needs to append the correct path to AVS SDK since it's not installed
	# to cross-compiler target sysroot
	set(ASDK_INCLUDE_DIRS "${ASDK_INCLUDE_DIRS}" "${CMAKE_PREFIX_PATH}/include")
	set(ASDK_LDFLAGS "${ASDK_LDFLAGS}" "-L${CMAKE_PREFIX_PATH}/lib")
else()
	# No package manager support in MSVC yet. Emulating the same results
	set(ASDK_INCLUDE_DIRS "${ASDK_INSTALL_PATH}/include")
	set(ASDK_LIB_DIR "${ASDK_INSTALL_PATH}/lib")
	set(SQLITE_INCLUDE_DIRS "${DEPS_PATH}/include")

	message (WARNING "SQLITE_INCLUDE_DIRS ${SQLITE_INCLUDE_DIRS} ASDK_INCLUDE_DIRS ${ASDK_INCLUDE_DIRS}" )
	set(ASDK_LDFLAGS
		"${ASDK_LIB_DIR}/ACL.lib"
		"${ASDK_LIB_DIR}/acsdkAlerts.lib"
		"${ASDK_LIB_DIR}/acsdkAudioPlayer.lib"
		"${ASDK_LIB_DIR}/acsdkBluetooth.lib"
		"${ASDK_LIB_DIR}/acsdkCBLAuthorizationDelegate.lib"
		"${ASDK_LIB_DIR}/acsdkCore.lib"
		"${ASDK_LIB_DIR}/acsdkDefaultDeviceSettingsManager.lib"
		"${ASDK_LIB_DIR}/acsdkDefaultInternetConnectionMonitor.lib"
		"${ASDK_LIB_DIR}/acsdkDeviceSetup.lib"
		"${ASDK_LIB_DIR}/acsdkDoNotDisturb.lib"
		"${ASDK_LIB_DIR}/acsdkEqualizer.lib"
		"${ASDK_LIB_DIR}/acsdkEqualizerImplementations.lib"
		"${ASDK_LIB_DIR}/acsdkExternalMediaPlayer.lib"
		"${ASDK_LIB_DIR}/acsdkExternalMediaPlayerInterfaces.lib"
		"${ASDK_LIB_DIR}/acsdkInteractionModel.lib"
		"${ASDK_LIB_DIR}/acsdkLibcurlHTTPContentFetcher.lib"
		"${ASDK_LIB_DIR}/acsdkLibcurlAlexaCommunications.lib"
		"${ASDK_LIB_DIR}/acsdkManufactory.lib"
		"${ASDK_LIB_DIR}/acsdkMultiRoomMusic.lib"
		"${ASDK_LIB_DIR}/acsdkNotifications.lib"
		"${ASDK_LIB_DIR}/acsdkNullMetricRecorder.lib"
		"${ASDK_LIB_DIR}/acsdkNullSpeechEncoder.lib"
		"${ASDK_LIB_DIR}/acsdkNullSystemTimeZone.lib"
		"${ASDK_LIB_DIR}/acsdkPostConnectOperationProviderRegistrar.lib"
		"${ASDK_LIB_DIR}/acsdkShared.lib"
		"${ASDK_LIB_DIR}/acsdkShutdownManager.lib"
		"${ASDK_LIB_DIR}/acsdkStartupManager.lib"
		"${ASDK_LIB_DIR}/acsdkSystemClockMonitor.lib"
		"${ASDK_LIB_DIR}/ADSL.lib"
		"${ASDK_LIB_DIR}/AFML.lib"
		"${ASDK_LIB_DIR}/AIP.lib"
		"${ASDK_LIB_DIR}/Alexa.lib"
		"${ASDK_LIB_DIR}/ApiGateway.lib"
		"${ASDK_LIB_DIR}/AudioResources.lib"
		"${ASDK_LIB_DIR}/AVSCommon.lib"
		"${ASDK_LIB_DIR}/AVSSystem.lib"
		"${ASDK_LIB_DIR}/CapabilitiesDelegate.lib"
		"${ASDK_LIB_DIR}/Captions.lib"
		"${ASDK_LIB_DIR}/CaptionsComponent.lib"
		"${ASDK_LIB_DIR}/CaptionsLib.lib"
		"${ASDK_LIB_DIR}/CBLAuthDelegate.lib"
		"${ASDK_LIB_DIR}/CertifiedSender.lib"
		"${ASDK_LIB_DIR}/ContextManager.lib"
		"${ASDK_LIB_DIR}/DefaultClient.lib"
		"${ASDK_LIB_DIR}/DeviceSettings.lib"
		"${ASDK_LIB_DIR}/Endpoints.lib"
		"${ASDK_LIB_DIR}/InterruptModel.lib"
		"${ASDK_LIB_DIR}/KWD.lib"
		"${ASDK_LIB_DIR}/PlaybackController.lib"
		"${ASDK_LIB_DIR}/RegistrationManager.lib"
		"${ASDK_LIB_DIR}/SDKComponent.lib"
		"${ASDK_LIB_DIR}/SoftwareComponentReporter.lib"
		"${ASDK_LIB_DIR}/SpeakerManager.lib"
		"${ASDK_LIB_DIR}/SpeechEncoder.lib"
		"${ASDK_LIB_DIR}/SpeechSynthesizer.lib"
		"${ASDK_LIB_DIR}/SQLiteStorage.lib"
		"${ASDK_LIB_DIR}/SynchronizeStateSender.lib"
		"${ASDK_LIB_DIR}/SystemSoundPlayer.lib"
		"${ASDK_LIB_DIR}/TemplateRuntime.lib")

endif()

include(build/BuildDefaults.cmake)
include(tools/Testing.cmake)

# Set variables for target install and .pc pkg-config file
include(build/cmake/PrepareInstall.cmake)

# Alexa Client SDK targets.
add_subdirectory("ThirdParty")
add_subdirectory("SmartScreenCapabilityAgents")
add_subdirectory("SmartScreenSDKInterfaces")
add_subdirectory("ApplicationUtilities")
add_subdirectory("APLClientLibrary")
if (NOT MSVC)
	# For UWP SampleApp files are built as part of UWPSampleApp
	add_subdirectory("SampleApp")
	add_subdirectory("doc")
endif()
add_subdirectory("Utils")
add_subdirectory("SSSDKCommon")

# Create .pc pkg-config file
include(build/cmake/GeneratePkgConfig.cmake)
