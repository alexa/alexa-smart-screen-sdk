### v2.0.2 released 04/14/2019:

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

