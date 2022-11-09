# Alexa Smart Screen SDK

<p>
    <a href="https://github.com/alexa/alexa-smart-screen-sdk/tree/v2.9.3" alt="version">
        <img src="https://img.shields.io/badge/stable%20version-2.9.3-brightgreen" /></a>
    <a href="https://github.com/alexa/avs-device-sdk/tree/v1.26.0" alt="DeviceSDK">
        <img src="https://img.shields.io/badge/avs%20device%20sdk-1.26.0-blueviolet" /></a>
    <a href="https://github.com/alexa/apl-client-library/tree/v1.8.3" alt="APLClientLibrary">
        <img src="https://img.shields.io/badge/apl%20client%20library-1.8.3-blue" /></a>
    <a href="https://github.com/alexa/alexa-smart-screen-sdk/issues" alt="issues">
        <img src="https://img.shields.io/github/issues/alexa/alexa-smart-screen-sdk" /></a>
</p>

## Announcement

> The new [Alexa Voice Service (AVS) Device SDK version 3.0](https://github.com/alexa/avs-device-sdk) and the [Alexa Smart Screen Web Components version 1.0](https://github.com/alexa/alexa-smart-screen-web-components) are now available!

> If you’re new: The AVS Device SDK version 3.0 and the Alexa Smart Screen Web Components have replaced the Alexa Smart Screen SDK.

> For current users of the Alexa Smart Screen SDK: Please consider upgrading to the AVS Device SDK version 3.0 and Alexa Smart Screen Web components.

The AVS Device SDK version 3.0 combines the Alexa Smart Screen SDK and the AVS Device SDK into one. It also adds several new features including [Video Device Control](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/video-device-control.html) for integrating [Video Skill Kit](https://developer.amazon.com/en-US/docs/alexa/video/video-skill-api-reference.html) capabilities into your device. To begin, see [Get Started with Alexa Voice Service Device SDK](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/overview.html).

The Alexa Smart Screen Web Components library is a separate Node.js library for smart screen device makers who build web apps that interact with the [AVS Device SDK](https://github.com/alexa/avs-device-sdk). For an overview, see [About Alexa Smart Screen Web Components](https://developer.amazon.com/en-US/docs/alexa/web-components/wc-about-web-components.html).

---

The [Alexa Smart Screen SDK](https://developer.amazon.com/alexa-voice-service/alexa-smart-screen-sdk) extends the [AVS Device SDK](https://developer.amazon.com/alexa-voice-service/sdk) to support development for screen-based Alexa Built-in products. This SDK enables device makers to build screen-based products that complement Alexa voice responses with rich visual experiences. 

The Alexa Smart Screen SDK package in this GitHub repo includes:
* The Alexa Smart Screen SDK
* A sample app that demonstrates end-to-end Alexa Smart Screen SDK functionality
* A GUI web app that handles presentation of Alexa visual responses

The Alexa Smart Screen SDK depends on the following additional GitHub repos:
* [AVS Device SDK](https://github.com/alexa/avs-device-sdk/tree/v1.26.0)
* [APL Client Library](https://github.com/alexa/apl-client-library/tree/v1.8.3)

## Note on APL Client Library

> When building the Alexa Smart Screen SDK, the npm registry may alert you to a security vulnerability with the package ```apl-client```.
> 
> The ```apl-client``` package for which this alert is raised is unrelated to the APL Client Library package that is part of the Alexa Smart Screen SDK.

## Get Started

You can set up the Alexa Smart Screen SDK by using the following Quick Start Guides:
* [MacOS Quick Start Guide](https://developer.amazon.com/en-US/docs/alexa/alexa-smart-screen-sdk/mac-os.html)
* [Raspberry Pi Quick Start Guide](https://developer.amazon.com/en-US/docs/alexa/alexa-smart-screen-sdk/raspberry-pi.html)
* [Ubuntu Quick Start Guide](https://developer.amazon.com/en-US/docs/alexa/alexa-smart-screen-sdk/ubuntu.html)

You can also create your device prototype by using an [Amazon-qualified development kit](https://developer.amazon.com/en-US/alexa/alexa-voice-service/dev-kits) that supports the Smart Screen SDK, such as:
* [Amazon Alexa Smart Screen Dev Kit](https://developer.amazon.com/alexa/alexa-voice-service/dev-kits/amazon-smart-screen)
* [Broadcom BCM972180 Voice STB Development Kit for Amazon AVS](https://www.broadcom.com/products/broadband/cable/reference-design/bcm972180_voice)

## SDK Architecture

This diagram illustrates the data flows between components that comprise the Alexa Smart Screen SDK.

 ![](https://m.media-amazon.com/images/G/01/mobile-apps/dex/sssdk/Alexa-smart-screen-sdk-detailed-component._TTH_.png)
 
**AVS Device SDK**: The SDK for Alexa Voice Service, which implements Alexa intelligent voice control functionality.

**Capability Agents (CAs)**: Handle Alexa-driven interactions; specifically directives and events. Each capability agent corresponds to a specific interface exposed by the Alexa Smart Screen API. These interfaces include:

* **AlexaPresentation** - The interface for rendering Alexa Presentation Language (APL) documents.
* **TemplateRuntime** - The interface for PlayerInfo and Template cards (such as music cards and other static cards).
* **VisualCharacteristics** - The interface for reporting the physical properties of visual display devices (such as screen resolution, window size, and supported interaction modes).

**Smart Screen Sample Application**: This is the Alexa Smart Screen SDK's sample app. It has two components: a sample app and a Javascript-based GUI client app.

* **GUIManager**: Interface between the capability agents and the GUI
* **GUIClient**: Manages messages between the GUI and the rest of the sample app
* **WebSocketServer**: Reference implementation of a WebSocket-based messaging interface

**APL Client**: An Inter-process Communication (IPC) based APL Runtime which implements the APL Core Engine and the IPC APL Web ViewHost to render APL payloads.

**HTML Rendering Engine**: A Javascript-based GUI client app and APL view host that render the visual experience on the device.

* **WebSocket**: Reference implementation of a WebSocket client
* **APL Web ViewHost**: Renders the APL document generated by the APL Core Library
* **Alexa GUI**: Reference GUI implementation 

## Security Requirements

All Alexa products must meet the [AVS Security Requirements](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html). In addition, when building the AVS Device SDK, you are required to adhere to the [following security principles](https://developer.amazon.com/en-US/docs/alexa/avs-device-sdk/overview.html#security-requirements).

## Important Considerations

* Review the Alexa Smart Screen [License](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/LICENSE.TXT).
* Review the Alexa Smart Screen [Notice about Third-Party Components](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/NOTICE.txt).

## Optional Configurations

### Add voice chrome

The default implementation provides information on [Alexa state](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/SDK-GUI-API.md#alexastatechanged), which you can use to create voice chrome. Be sure to follow the [AVS Voice Chrome guidelines](https://developer.amazon.com/docs/alexa-voice-service/ux-design-attention.html#chrome).

### Run the GUI client with predefined device visual characteristics and GUI client configurations

We provide four different sample configuration files containing predefined device visual characteristics and GUI client configurations. These can be found under `modules/GUI/config/guiConfigSamples`.
You can pass any of them as an extra config file argument after the main Smart Screen SDK config file argument when running the Sample App, for example:
```
    cd <pathTo>/ss-build
    ./modules/Alexa/SampleApp/src/SampleApp
        -C <pathTo>/sdk-build/Integration/AlexaClientSDKConfig.json
        -C <pathTo>/alexa-smart-screen-sdk/modules/GUI/config/SmartScreenSDKConfig.json
        -C <pathTo>/alexa-smart-screen-sdk/modules/GUI/config/guiConfigSamples/GuiConfigSample_TvOverlayPortrait.json 
        -L INFO
```

### Remote control support

Functionality for Exit and Back buttons (as found on a device's physical remote control) is minimally supported by the Smart Screen SDK. The following behaviors are expected to occur on execution of either a `BACK` or `EXIT` [navigationEvent](./modules/GUI/SDK-GUI-API.md#navigationevent):

* Clear the rendering screen - Exit fully out of the Alexa-presented display so that no static image or layout is left.
* Release the focus management - Release any focus management that might be held.

Using the default gui client configuration's [device keys](./modules/GUI/config/SmartScreenSDKConfig.md#device-keys-parameters), `Esc` and `B` are mapped to `EXIT` and `BACK` respectively.
