### v2.5.0 released 02/01/2021:
#### Enhancements
* Upgraded dependency to Device SDK 1.22.0 [https://github.com/alexa/avs-device-sdk/tree/v1.22.0]
* Added support for multiple concurrent APL clients
* Added support for APL 1.5 [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-latest-version.html]
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
* Added support for APL Extensions [https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-latest-version.html#apl-extensions-and-the-backstack]
 
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

