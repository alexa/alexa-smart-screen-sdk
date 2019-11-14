/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_JSONUIMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_JSONUIMANAGER_H_

#include <memory>
#include <string>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/NotificationsObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SingleSettingObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerObserverInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <CBLAuthDelegate/CBLAuthRequesterInterface.h>

#include <SmartScreenSDKInterfaces/GUIClientInterface.h>
#include <SmartScreenSDKInterfaces/UIManagerInterface.h>
#include <SmartScreenSDKInterfaces/MessagingServerObserverInterface.h>

#include "Messages/GUIClientMessage.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * This class manages printing states to the console that the user will see when interacting with
 * the Sample Application.
 */
class JsonUIManager
        : public smartScreenSDKInterfaces::UIManagerInterface
        , public smartScreenSDKInterfaces::MessagingServerObserverInterface {
public:
    /**
     * Constructor.
     */
    JsonUIManager(
        std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClientInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo);

    void onDialogUXStateChanged(DialogUXState state) override;

    void onConnectionStatusChanged(const Status status, const ChangedReason reason) override;

    void onSettingChanged(const std::string& key, const std::string& value) override;

    /// @name SpeakerManagerObserverInterface Functions
    /// @{
    void onSpeakerSettingsChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerObserverInterface::Source& source,
        const alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type& type,
        const alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings& settings) override;
    /// }

    /// @name NotificationsObserverInterface Functions
    /// @{
    void onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState state) override;

    void onNotificationReceived() override{};
    /// }

    /// @name CBLAuthRequesterInterface Functions
    /// @{
    void onRequestAuthorization(const std::string& url, const std::string& code) override;
    void onCheckingForAuthorization() override;
    /// }

    /// @name AuthObserverInterface Methods
    /// @{
    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error newError) override;
    /// }

    /// @name CapabilitiesObserverInterface Methods
    /// @{
    void onCapabilitiesStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State newState,
        alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error newError) override;
    /// }

    /// @name MessagingServerObserverInterface Methods
    /// @{
    void onConnectionOpened() override;
    void onConnectionClosed() override;
    /// }

    /**
     * Prints the welcome screen.
     */
    void printWelcomeScreen() override;

    /**
     * Prints the help screen.
     */
    void printHelpScreen() override;

    /**
     * Prints the help screen with limited options. This is used when not connected to AVS.
     */
    void printLimitedHelp() override;

    /**
     * Prints the Settings Options screen.
     */
    void printSettingsScreen() override;

    /**
     * Prints the Locale Options screen.
     */
    void printLocaleScreen() override;

    /**
     * Prints the Speaker Control Options screen. This prompts the user to select a @c SpeakerInterface::Type to modify.
     */
    void printSpeakerControlScreen() override;

    /**
     * Prints the Firmware Version Control screen. This prompts the user to enter a positive decimal integer.
     */
    void printFirmwareVersionControlScreen() override;

    /**
     * Prints the Volume Control Options screen. This gives the user the possible volume control options.
     */
    void printVolumeControlScreen() override;

    /**
     * Prints the ESP Control Options screen. This gives the user the possible ESP control options.
     */
    void printESPControlScreen(bool support, const std::string& voiceEnergy, const std::string& ambientEnergy) override;

    /**
     * Prints the Comms Control Options screen. This gives the user the possible Comms control options.
     */
    void printCommsControlScreen() override;

    /**
     * Prints the Error Message for Wrong Input.
     */
    void printErrorScreen() override;

    /**
     * Notifies the user that the microphone is off.
     */
    void microphoneOff() override;

    /*
     * Prints the state that Alexa is currenty in.
     */
    void microphoneOn() override;

    /**
     * Prints a warning that the customer still has to manually deregister the device.
     */
    void printResetWarning() override;

    /**
     * Prints a confirmation message prompting the user to confirm their intent.
     */
    void printResetConfirmation() override;

    /**
     * Prints a confirmation message prompting the user to confirm their intent to reauthorize the device.
     */
    void printReauthorizeConfirmation() override;

    /**
     * Prints an error message while trying to configure ESP in a device where ESP is not supported.
     */
    void printESPNotSupported() override;

    /**
     * Prints an error message while trying to override ESP Data in a device that do not support manual override.
     */
    void printESPDataOverrideNotSupported() override;

    /**
     * Prints an error message when trying to access Comms controls if Comms is not supported.
     */
    void printCommsNotSupported() override;

private:
    /**
     * Prints the current state of Alexa after checking what the appropriate message to display is based on the current
     * component states. This should only be used within the internal executor.
     */
    void reportAlexaState();

    /**
     * Sets the failure status. If status is new and not empty, we'll print the limited mode help.
     *
     * @param failureStatus Status message with the failure reason.
     * @warning Only call this function from inside the executor thread.
     */
    void setFailureStatus(const std::string& status);

    /**
     * Sends authorization request message to GUI Client Interface.
     */ 
    void sendAuthorizationRequestMessage();

    /// The current dialog UX state of the SDK
    DialogUXState m_dialogState;

    /// The current CapabilitiesDelegate state.
    alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State m_capabilitiesState;

    /// The error associated with the CapabilitiesDelegate state.
    alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error m_capabilitiesError;

    /// The current authorization state of the SDK.
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState;

    /// Counter used to make repeated messages about checking for authorization distinguishable from each other.
    int m_authCheckCounter;

    /// The current connection state of the SDK.
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // String that holds a failure status message to be displayed when we are in limited mode.
    std::string m_failureStatus;

    /// Pointer to the GUI Client interface
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> m_guiClientInterface;

    // URL returned by CBLAuthRequesterInterface for authorization
    std::string m_authUrl;
    
    // CBL Code returned by CBLAuthRequesterInterface for authorization
    std::string m_authCode;

    /// DeviceInfo object for reporting config information
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_JSONUIMANAGER_H_
