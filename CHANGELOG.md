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

