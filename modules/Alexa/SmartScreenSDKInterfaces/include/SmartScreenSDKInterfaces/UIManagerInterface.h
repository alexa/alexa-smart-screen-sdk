/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_UIMANAGERINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_UIMANAGERINTERFACE_H

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/NotificationsObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SingleSettingObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerObserverInterface.h>
#include <CBLAuthDelegate/CBLAuthRequesterInterface.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

class UIManagerInterface
        : public virtual alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::SingleSettingObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerObserverInterface
        , public virtual alexaClientSDK::avsCommon::sdkInterfaces::NotificationsObserverInterface
        , public virtual alexaClientSDK::authorization::cblAuthDelegate::CBLAuthRequesterInterface {
public:
    virtual ~UIManagerInterface() = default;

    /**
     * Prints the welcome screen.
     */
    virtual void printWelcomeScreen() = 0;

    /**
     * Prints the help screen.
     */
    virtual void printHelpScreen() = 0;

    /**
     * Prints the help screen with limited options. This is used when not connected to AVS.
     */
    virtual void printLimitedHelp() = 0;

    /**
     * Prints the Settings Options screen.
     */
    virtual void printSettingsScreen() = 0;

    /**
     * Prints the Locale Options screen.
     */
    virtual void printLocaleScreen() = 0;

    /**
     * Prints the Speaker Control Options screen. This prompts the user to select a @c SpeakerInterface::Type to modify.
     */
    virtual void printSpeakerControlScreen() = 0;

    /**
     * Prints the Firmware Version Control screen. This prompts the user to enter a positive decimal integer.
     */
    virtual void printFirmwareVersionControlScreen() = 0;

    /**
     * Prints the Volume Control Options screen. This gives the user the possible volume control options.
     */
    virtual void printVolumeControlScreen() = 0;

    /**
     * Prints the ESP Control Options screen. This gives the user the possible ESP control options.
     */
    virtual void printESPControlScreen(
        bool support,
        const std::string& voiceEnergy,
        const std::string& ambientEnergy) = 0;

    /**
     * Prints the Comms Control Options screen. This gives the user the possible Comms control options.
     */
    virtual void printCommsControlScreen() = 0;

    /**
     * Prints the Error Message for Wrong Input.
     */
    virtual void printErrorScreen() = 0;

    /**
     * Notifies the user that the microphone is off.
     */
    virtual void microphoneOff() = 0;

    /*
     * Prints the state that Alexa is currenty in.
     */
    virtual void microphoneOn() = 0;

    /**
     * Prints a warning that the customer still has to manually deregister the device.
     */
    virtual void printResetWarning() = 0;

    /**
     * Prints a confirmation message prompting the user to confirm their intent.
     */
    virtual void printResetConfirmation() = 0;

    /**
     * Prints a confirmation message prompting the user to confirm their intent to reauthorize the device.
     */
    virtual void printReauthorizeConfirmation() = 0;

    /**
     * Prints an error message while trying to configure ESP in a device where ESP is not supported.
     */
    virtual void printESPNotSupported() = 0;

    /**
     * Prints an error message while trying to override ESP Data in a device that do not support manual override.
     */
    virtual void printESPDataOverrideNotSupported() = 0;

    /**
     * Prints an error message when trying to access Comms controls if Comms is not supported.
     */
    virtual void printCommsNotSupported() = 0;
};
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK
#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_UIMANAGERINTERFACE_H
