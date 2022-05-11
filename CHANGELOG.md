### v2.9.2 released 05/11/2022:
#### Enhancements
* Upgraded async version to 2.6.4. Note that users upgrading to this version will need to remove the existing node modules from modules/GUI/js/node_modules before the build.

### v2.9.1 released 04/05/2022:
#### Bug fixes
* Fixed hold-to-talk event handling logic in the native layer to keep it in sync with GUI

#### Enhancements
* Upgraded dependency to APL Client Library 1.8.3
* Upgraded nanoid version to 3.2.0, minimist version to 1.2.6 and refreshed the package-lock.json with new dependencies. Note that users upgrading to this version will need to remove the existing node modules from modules/GUI/js/node_modules before the build.

### v2.9.0 released 12/16/2021:
#### Breaking Changes
* Decoupled APL Client Library and APL Core Engine build from Smart Screen SDK. Please follow new build instructions here [https://developer.amazon.com/en-US/docs/alexa/alexa-smart-screen-sdk/overview.html]

#### Enhancements
* Upgraded dependency to Device SDK 1.26.0 [https://github.com/alexa/avs-device-sdk/tree/v1.26.0]
* Added support for APL 1.8 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-latest-version.html]
* Generalized audio focus management to support additional AVS interfaces in addition to Alexa.Presentation.APL
* Added support for window display state reporting to APL Renderer
* Adopted APL version API to be able to dynamically obtain dependency APL version
* Added additional validation logic for SmartScreenSDKConfig.json file
* Upgraded websocketpp dependency from v0.8.1 to v0.8.2
* Refreshed the package-lock.json with new dependencies. Note that users upgrading to this version will need to remove the existing node modules from modules/GUI/js/node_modules before the build.

### v2.8.0 released 09/07/2021:
#### Breaking Changes
* APL Client Library has been extracted into separate GitHub repo [https://github.com/alexa/apl-client-library/tree/v1.7.2]. Users will need to pull the APL Client Library to build the Smart Screen SDK.

#### Enhancements
* Upgraded dependency to Device SDK 1.25.0 [https://github.com/alexa/avs-device-sdk/tree/v1.25.0]
* Added support for APL 1.7 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-changes-1-7.html]
* Captions support for right-to-left languages
* Upgraded node version requirement to 14.17.4
* Upgraded typescript version to 3.4 and refreshed the package-lock.json with new dependencies. Note that users upgrading to this version will need to remove the existing node modules from modules/GUI/js/node_modules before the build.

### v2.7.1 released 07/23/2021:
#### Bug fixes
* Upgraded dependency to APL Core Library 1.6.2 Release [https://github.com/alexa/apl-core-library/releases/tag/v1.6.2]
* Fixed issue with some video skills not playing due to AVS Video stealing audio focus before speechlet gets to finish
* Fixed issue with APL cards not rendering due to APLCoreEngine content being corrupted during serialization
* Fixed issue with RenderPlayerInfo card play/pause/right/left buttons not working and play button being not focused on initial render

### v2.7.0 released 06/01/2021:
#### Enhancements
* Upgraded dependency to Device SDK 1.24.0 [https://github.com/alexa/avs-device-sdk/tree/v1.24.0]
* Added support for APL 1.6 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-changes-1-6.html] 
* Added support for Alexa.Presentation.APL 1.3 [https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#version-changes]
* Added support for timeoutType from Alexa.Presentation.APL RenderDocument directives [https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#renderdocument]
* Moved metric payload handling logic into APLClientLibrary
* Removed user-specified schema path for SmartScreenSDKConfig.json validation in favor of internal schema file
* Hardened AlexaPresentation CA logic to align with the existing limitation of only being able to support one active APL directive document at a time
* Upgraded Node version requirement to 14.0, added Node version check
* Added warning message when building in DEBUG mode

#### Bug fixes
* Fixed issue that RenderPlayerInfo card does not get rendered until the previously displayed APL card times out.
* Fixed issue that AlexaPresentation CA sends incorrect IPC messages to GUI App, causing GUI App to maintain an incorrect window state.

### v2.6.0 released 04/01/2021:
#### Enhancements
* Upgraded dependency to Device SDK 1.23.0 [https://github.com/alexa/avs-device-sdk/tree/v1.23.0]
* Added support for video calling with Alexa Communication. Customers who want to enable this feature should contact their Alexa representative. Video calling is currently only supported on Linux devices.
* Added support for SmartScreenSDKConfig.json validation for DCMAKE_BUILD_TYPE=DEBUG mode with SampleApp runtime parameter '-S <path-to-ss-sdk-schema-file>'
* Improved the SDK handling for rendering directives for invalid window id
* Updated webpack version to 5.28.0 and refreshed the package-lock.json with new dependencies. Note that users upgrading
  to this version will need to remove the existing node modules from `modules/GUI/js/node_modules` before the build.

#### Bug fixes
* Added support to handle document with incorrect windowId gracefully
* Fixed issue with audio player still playing after user barge-in clears the screen during transitions
* Fixed issues with Finish APL ExecuteCommand ending all ongoing activities
* Fixed intermittent issue with Audio Player playing queued up speakItem for inactive window
* Fixed issues related to uninitialized scalar, uncaught exceptions, big parameter pass-by-value and unchecked return values

### v2.5.0 released 02/01/2021:
#### Enhancements
* Upgraded dependency to Device SDK 1.22.0 [https://github.com/alexa/avs-device-sdk/tree/v1.22.0]
* Added support for multiple concurrent APL clients
* Added support for APL 1.5 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-changes-1-5.html]
* Ported DISABLE_DUCKING cmake configuration from Alexa-device-sdk
* Music playback now ducks instead of pauses when APL TTS is executed
* Updated ini patch version to 1.3.6

#### Bug fixes
* Fixed issue with video in `disallowVideo` mode
* Fixed race condition where GUIManager tries to access SmartScreenClient before setting it
* Fixed race condition while configuring notification settings in GUIManager, this was causing the sampleapp to crash
* Increased Caption's font size and modified its z order to display it on top
* Fixed the issue with typescript wrongly resolving setTimeout to nodeJS version without "window"
* Fixed the issue with speech recognition is not interrupting APL activity
* Fixed a regression introduced in 2.4 wherein keyboard events no longer interrupted TTS and APL activity

### v2.4.0 released 11/09/2020:
#### Enhancements
* Upgraded dependency to Device SDK 1.21.0 [https://github.com/alexa/avs-device-sdk/tree/v1.21.0]
* Added DND Support
 
#### Bug fixes
* Fixed a back navigation issue causing a RenderPlayerInfo card to be cleared
* Fixed an issue where the sample app automatically goes into the listening mode when prompted by a skill

### v2.3.0 released 09/24/2020:
#### Enhancements
* Upgraded dependency to Device SDK 1.20.1 [https://github.com/alexa/avs-device-sdk/tree/v1.20.1]
* Support for APL 1.4 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-changes-1-4.html]
* Added APL Telemetry Support
* TV Overlay Portrait is no longer supported
* Added support for APL Extensions [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-extensions-v1-4.html]
 
#### Bug fixes
* Correctly handle GUI state when presenting content over attenuated PlayerInfo card
* Lock JS dependency version - using package-lock.json
* Fixed alexa state on touch event
* Fixed import content failure cache update

### v2.2.0 released 07/13/2020:
#### Enhancements
* Upgraded dependency to Device SDK 1.20.0
* Optimization for APL package import latency

#### Bug fixes
* Clean shutdown for device de-registration/logout
* Proper scaling for GUI so entire window is visible
* Fixed proper setting of displayCardTTSFinishedTimeout
* Notification sound cues are now playing

#### Known issues
* Display of captions is not in sync with Alexa audio and is as expected
* Memory leak issue with resetDevice member function in GUIManager
* Potential memory leak issue with executeCommands directive
* Skill session remains alive for a short amount of time after exit
* Buttons on Now Playing display card do not update their visual states when updated by voice
* Scrolling on General Knowledge display card does not work when the content does not fit into screen
* Progress bar on Now Playing display card does not reflect the actual audio offset when such information is not provided in RenderPlayerInfo directive

#### Notes
* The AVS Device SDK must be built with an additional flag: `-DRAPIDJSON_MEM_OPTIMIZATION=OFF`

### v2.1.0 released 06/02/2020:
#### Enhancements
* Upgraded dependency to Device SDK 1.19.1
* Provided a reference implementation for rendering of captions
* Support for APL 1.3
* Added support for visual metrics
* Moved APL Core integration out of SampleApp into a standalone library

#### Bug fixes
* Fixed browser warnings caused by short-hand definition of "transition" property
* Fixed potentially invalid and corrupted data due to using multiple rapidjson allocators

#### Known issues
* SampleApp screen cut off at the bottom on fullscreen when using emulateDisplayDimensions configuration
* Display of captions is not in sync with Alexa audio and is as expected
* Memory leak issue with resetDevice member function in GUIManager
* Notification cues are not playing
* Potential memory leak issue with executeCommands directive
* Skill session remains alive for a short amount of time after exit
* Control buttons on the Flash briefing page is disabled
* Buttons on Now Playing display card do not update their visual states when updated by voice
* Scrolling on General Knowledge display card does not work when the content does not fit into screen
* Progress bar on Now Playing display card does not reflect the actual audio offset when such information is not provided in RenderPlayerInfo directive 

### v2.0.2 released 04/14/2020:
#### Bug fixes
* Added support for Raspbian Buster.
* Upgrade dependency on Device SDK 1.19
* Player info no longer waits for the TTS timeout to elapse before being displayed.
* Fixed a bug with some objects not being properly cleaned-up during shutdown.
* Timer sounds now stop correctly when dialog is interrupted.
* Minor fixes in rendering (text truncations for non-English languages, emoji handling, hyphenation).
* Fixed image scaling and shadows related bug. 
* Fixed AVG rendering bug.
* Fixed a HLS Playing bug.
* Improved APL imports processing latency.

#### Known issues:
* For some videos with sound, there might be an issue associated with auto-playing such videos in both Chrome and Firefox. Permission changes in browsers are needed for auto-playing those videos.  
* The build option ENABLE_CAPTIONS does not enable captions. Enabling it will cause undefined behaviour during a build.

### v2.0.1 released 11/26/2019:
#### Bug Fixes
* Fixed segfault is situation when bad Misc DB path was provided in configuration.
* Fixed situation where video playback controls were unresponsive on some skills.
* Audio now plays without highlighting if text extraction fails during SpeakItem/SpeakList command.

#### Known Issues
* Memory of some objects may leak when restarting the SampleApp object. This doesn't happen with vanila SampleApp provided with the SDK as we don't expose logout to the GUI layer. Until we fix this, don't add new cases where `SampleApp::run` method will return `SampleApplicationReturnCode::RESTART`.
* In rare cases button presses could trigger repeat speech.

### v2.0 released 11/16/2019:
Initial public release

#### Known Issues
* Successive SpeakItem commands can block command execution
* Music cards occasionally not cleared after timeout
* Music card toggle controls not highlighted by voice interactions
* Music card may not display in APL skills
* Focus manager prints errors to console on some skills
* Video playback controls unresponsive on some skills
* Noise / Blur filters do not work on Firefox
* 'Go Home' is not supported

