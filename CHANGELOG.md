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

