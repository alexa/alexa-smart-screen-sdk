# Alexa Smart Screen SDK Config Specification

The SmartScreenSDKConfig is used to configure additional functionality of the SmartScreenSDK SampleApp and GUI App apart from the AlexaClientSDKConfig.  The config manages declaration of the device's visual characteristics capabilities and windows for rendering APL responses, as well as configuration of the reference websocket server used for communication between the SDK and GUI app.

The [default config](SmartScreenSDKConfig.json) file is a reference implementation of the SmartScreenSDK and GUI App for a TV type device that displays both fullscreen and lower third visual responses.

There are additional sample device configurations for the GUI in the /guiConfigSamples:
* [TV Fullscreen Only](guiConfigSamples/GuiConfigSample_TvFullscreen.json)
* [Smart Screen Landscape](guiConfigSamples/GuiConfigSample_SmartScreenLargeLandscape.json)
* [Smart Screen Round](guiConfigSamples/GuiConfigSample_SmartScreenRound.json)

# Config Parameters

```
{
  "sampleApp": {
    "websocketInterface":"{{STRING}}",
    "websocketPort":{{NUMBER}},
    "websocketCertificateAuthority":"{{STRING}}",
    "websocketCertificate":"{{STRING}}",
    "websocketPrivateKey":"{{STRING}}",
    "contentCacheReusePeriodInSeconds": "{{STRING}}",
    "contentCacheMaxSize": "{{STRING}}"
  },
  "alexaPresentationCapabilityAgent": {
    "minStateReportIntervalMs": {{NUMBER}},
    "stateReportCheckIntervalMs": {{NUMBER}}
  },
  "gui": {
    "appConfig": {
      "description":"{{STRING}}",
      "mode":{{DeviceMode}},
      "emulateDisplayDimensions":{{BOOLEAN}},
      "scaleToFill":{{BOOLEAN}},
      "audioInputInitiator":{{AudioInputInitiator}},
      "defaultWindowId":"{{STRING}}",
      "windows": [
        {{GUIWindow}},
        {{GUIWindow}},
        ...
      ],
      "deviceKeys": {{DeviceKeys}}
    },
    "visualCharacteristics": [
      {{Alexa.InteractionMode}},
      {{Alexa.Presentation.APL.Video}},
      {{Alexa.Display.Window}},
      {{Alexa.Display}}
    ]
  }
}
```

| Parameter                         | Type                                                                              | Required  | Description
| -------------                     |-------                                                                            |:-----:    | ----- |
| sampleApp                         | [SampleApp](#sampleapp-parameters)                                                | Yes       | Config for SDK SampleApp. 
| alexaPresentationCapabilityAgent  | [AlexaPresentationCapabilityAgent](#alexapresentationcapabilityagent-parameters)  | No        | Config for Alexa Presentation Capability Agent. 
| gui                               | [GUI](#gui-parameters)                                                            | Yes       | Config for GUI App and Device Visual Characteristic Capabilities. 

# SampleApp Parameters

Config parameters for the SmartScreenSDK SampleApp.  Handles configuration of  [websocket server](https://developer.amazon.com/en-US/docs/alexa/alexa-smart-screen-sdk/security-requirements.html#html-rendering-client-with-websocket-ipc-sample-app) used for [message communication between SDK SampleApp and GUI App](../SDK-GUI-API.md) and caching of imported packages.

```
"sampleApp": {
    "websocketInterface":"{{STRING}}",
    "websocketPort":{{NUMBER}},
    "websocketCertificateAuthority":"{{STRING}}",
    "websocketCertificate":"{{STRING}}",
    "websocketPrivateKey":"{{STRING}}",
    "contentCacheReusePeriodInSeconds": "{{STRING}}",
    "contentCacheMaxSize": "{{STRING}}"
}
```

| Parameter                         | Type      | Required  | Default           | Description
| -------------                     |-------    |:--------: |-----              | ----- |
| websocketInterface                | string    | No        | `"127.0.0.1"`     | The interface which the websocket server will bind to.<br/><br/>**Note**: For security reasons, it is strongly recommended that the loopback interface is used.  When using interfaces, other than loopback, additional security measures should be taken to ensure the security and integrity of data between the client and server.
| websocketPort                     | number    | No        | `8933`            | The port which the websocket server will listen to.<br/><br/>**Note**: The port should be a positive integer in the range `[1-65535]`, It is strongly recommended that a port number `> 1023` is used
| websocketCertificateAuthority     | string    | No        | `"ca.cert"`       | The Certificate Authority file to verify client certificate.
| websocketCertificate              | string    | No        | `"server.chain"`  | The certificate file the websocket server should use when SSL is enabled.
| contentCacheReusePeriodInSeconds  | string    | No        | `"600"`           | The number of seconds to reuse a cached package.
| contentCacheMaxSize               | string    | No        | `"50"`            | The max size for the cache of imported packages.

# AlexaPresentationCapabilityAgent Parameters

Config parameters for the Alexa Presentation Capability Agent. Handles configuration of state reporting for APL context [RenderedDocumentState](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#rendereddocumentstate). Use of these parameters will cause the Alexa Presentation CA to proactively report states to the cloud even when there are no scheduled events. This behavior is disabled when these parameters are not set or when the state report check interval is set to `0`.

```
"alexaPresentationCapabilityAgent": {
    "minStateReportIntervalMs": {{NUMBER}},
    "stateReportCheckIntervalMs": {{NUMBER}}
}
```

| Parameter                   | Type      | Required  | Default     | Description
| -------------               |-------    |:--------: |-----        | ----- |
| minStateReportIntervalMs    | number    | No        | `600`       | The minimum state reporting interval in milliseconds for the AlexaPresentation CA.
| stateReportCheckIntervalMs  | number    | No        | `0`         | The interval between checks for context changes.

# GUI Parameters

Config parameters used to define the device's reported visual characteristic capabilities, and the GUI app's available windows and supported features for displaying APL responses.

```
"gui": {
    "appConfig": {
      "description":"{{STRING}}",
      "mode":{{DeviceMode}},
      "emulateDisplayDimensions":{{BOOLEAN}},
      "scaleToFill":{{BOOLEAN}},
      "audioInputInitiator":{{AudioInputInitiator}},
      "defaultWindowId":"{{STRING}}",
      "windows": [
        {{GUIWindow}},
        {{GUIWindow}},
        ...
      ],
      "deviceKeys": {{DeviceKeys}}
    },
    "visualCharacteristics": [
      {{Alexa.InteractionMode}},
      {{Alexa.Presentation.APL.Video}},
      {{Alexa.Display.Window}},
      {{Alexa.Display}}
    ]
}
```

| Parameter             | Type                                                          | Required  | Description
| -------------         |-------                                                        |:-----:    | ----- |
| appConfig             | [appConfig](#appconfig-parameters)                            | Yes       | Config for GUI App functionality and windows. 
| visualCharacteristics | [visualCharacteristics](#visual-characteristics-parameters)   | Yes       | Config for device-reported visual characteristics.

## AppConfig Parameters
Parameters for GUI App and APL window functionality.

| Parameter                     | Type                                      | Required  | Default       | Description
| -------------                 |-------                                    |:-----:    | ------------- | ----- |
| description                   | string                                    | No        |  None         | Description of device to present in GUI App `SampleHome` screen.
| mode                          | string                                    | No        | `HUB`         | The expected [DeviceMode](https://developer.amazon.com/docs/alexa-presentation-language/apl-viewport-property.html#viewport_mode_property) of the device.<br/>Valid values: `'AUTO', 'HUB', 'MOBILE', 'PC', 'TV'`<br/><br/>**Note**: Currently ONLY `'HUB'` and `'TV'` are officially supported device modes.
| emulateDisplayDimensions      | boolean                                   | No        | `false`       | If true, the GUI app's root container will be explicitly sized to the device's display dimensions defined in visualCharacteristics `Alexa.Display`. 
| scaleToFill                   | boolean                                   | No        | `false`       | If true the GUI's app root container will be uniformly scaled to fit within the supplied window.
| audioInputInitiator           | string                                    | No        | `TAP`         | The [AudioInputInitiator](https://developer.amazon.com/docs/alexa-voice-service/speechrecognizer.html#initiator) type that will be messaged from the GUI app to the SampleApp for voice input.<br/>Valid values: `'PRESS_AND_HOLD', 'TAP', 'WAKEWORD'`
| defaultWindowId               | string                                    | Yes       | None          | The ID of the [APL Window](#apl-window-parameters) to use as the default for visual responses. *Note* the config for this window ID will also be used to create the PlayerInfo window for audio playback presentation.
| windows                       | [[APL Windows](#apl-window-parameters)]   | Yes       | None          | A z-ordered array of [APL Window](#apl-window-parameters) configurations to create in the GUI App and report in `Alexa.Display.Window.WindowState` for targeting and presentation of APL responses.  These windows can be targeted by ID.<br/>`0 index = z-order: 0, 1 index = z-order: 1, ...`
| deviceKeys                    | [Device Keys](#device-keys-parameters)    | Yes       | None          | The [Device Keys](#device-keys-parameters) config for defining core function keys in the GUI app relative to `talk, back, and exit` key input. 

### APL Window Parameters
Parameters for targetable APL windows in the GUI App.  The device's windows are defined and reported via [visualCharacteristics](#visual-characteristics-parameters) `Alexa.Display.Window`, and this config is used to reference those window definitions and configure additional runtime features of the APL viewhost running within the window.  

| Parameter             | Type      | Required      | Default   | Description
| -------------         |-------    |:-----:        | -----     |----- |
| id                    | string    | Yes           | None      | A unique identifier for the window instance which is used to target content to this window.
| templateId            | string    | Yes           | None      | Identifies the window template in the `Alexa.Display.Window` capability which is the basis for this instance.
| sizeConfigurationId   | string    | Yes           | None      | Indicates the active size configuration of the window as defined in `Alexa.Display.Window`. 
| interactionMode       | string    | Yes           | None      | An "id" value representing the current `Alexa.InteractionMode` of the window. 
| theme                 | string    | No            | `dark`    | Represents the preferred basic color scheme of the window.  Content developers may optionally use this value for [styling their visual response](https://developer.amazon.com/docs/alexa-presentation-language/apl-viewport-property.html#theme).<br/>Valid values: `'light'` and `'dark'`. 
| allowOpenUrl          | boolean   | No            | `false`   | Indicates if the window supports the APL [OpenUrlCommand](https://developer.amazon.com/docs/alexa-presentation-language/apl-standard-commands.html#open_url_command).
| animationQuality      | string    | No            | `normal`  | Indicates the level of [AnimationQuality](https://developer.amazon.com/docs/alexa-presentation-language/apl-data-binding-evaluation.html#animation) support in the window.<br/>Valid Values: `none = 0, slow = 1, normal = 2`.
| windowPosition        | string    | No            | `center`  | The screen position of the window.<br/>Valid values are `'center', 'right', 'left', 'top', 'bottom'`.  
| token                 | string    | No            | `null`    | The token identifying the content currently occupying the window.  This is set by the window targeting response.
| supportedExtensions   | [strings] | No            | `null`    | An optional array of [APL extension uri's](https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/rootconfig.h#L360) supported in the window instance.  Note that any extensions declared here must have been built with the APLClient extensions framework to be available to the APL runtime for the window.

### Device Keys Parameters
Config for device input keys.

| Parameter             | Type                                  | Required  | Description
| -------------         |-------                                |:-----:    | ----- |
| talkKey               | [Device Key](#device-key-parameters)  | Yes       | Key used in the GUI App to send either a [tapToTalk](../SDK-GUI-API.md#taptotalk) or [holdToTalk](../SDK-GUI-API.md#holdtotalk) message to the SDK based on `AudioInputInitiator` parameter defined in the [AppConfig](#appconfig-parameters).
| backKey               | [Device Key](#device-key-parameters)  | Yes       | Key used in the GUI App to send a [navigationEvent](../SDK-GUI-API.md#navigationevent) to the SDK with event type `BACK`.
| exitKey               | [Device Key](#device-key-parameters)  | Yes       | Key used in the GUI App to send a [navigationEvent](../SDK-GUI-API.md#navigationevent) to the SDK with event type `EXIT`.
| toggleCaptionsKey     | [Device Key](#device-key-parameters)  | Yes       | Key used in the GUI App to send a [toggleCaptionsEvent](../SDK-GUI-API.md#togglecaptionsevent) to the SDK.
| toggleDoNotDisturbKey | [Device Key](#device-key-parameters)  | Yes       | Key used in the GUI App to send a [toggleDoNotDisturb](../SDK-GUI-API.md#toggledonotdisturb) to the SDK.

#### Device Key Parameters
Config for individual device key.

| Parameter             | Type      | Required  | Description
| -------------         |-------    |:-----:    | ----- |
| code                  | string    | Yes       | Property representing a physical key on the keyboard, following the [KeyboardEvent.code](https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code) web standard.
| keyCode               | number    | Yes       | Property representing a system and implementation dependent numerical code identifying the unmodified value of the pressed key, following the [KeyboardEvent.keyCode](https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode) web standard.
| key                   | string    | Yes       | Property representing the value of the key pressed by the user, following the [KeyboardEvent.key](https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key) web standard.

## Visual Characteristics Parameters
Config for device reported visual characteristic capabilities. All config objects map explicitly to the capability's API configuration.

| Type                                                                                                              | Required  | Description
| -------------                                                                                                     |:-------:  |-----|
| [Alexa.Display](https://developer.amazon.com/docs/alexa-voice-service/display.html)                               | Yes       | A definition of the physical properties of two-dimensional video display devices, such as LCD displays, LED displays, etc..
| [Alexa.Display.Window](https://developer.amazon.com/docs/alexa-voice-service/display-window.html)                 | Yes       | A definition of the possible windows that may be created on this device's display. A display can contain multiple windows.
| [Alexa.InteractionMode](https://developer.amazon.com/docs/alexa-voice-service/interaction-mode.html)              | Yes       | A definition of the ways that a user can interact with this device.
| [Alexa.Presentation.APL.Video](https://developer.amazon.com/docs/alexa-voice-service/presentation-apl-video.html) | Yes       | A definition of the supported video codecs and playback abilities supported by the APL runtime on the device, which may differ from video codecs supported elsewhere on the device.