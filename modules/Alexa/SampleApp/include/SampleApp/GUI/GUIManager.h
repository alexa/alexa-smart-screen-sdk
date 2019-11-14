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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_

#include <Audio/MicrophoneInterface.h>
#include <AFML/FocusManager.h>
#include <AVSCommon/AVS/FocusState.h>
#include <AVSCommon/SDKInterfaces/AudioPlayerObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AlexaPresentation/AlexaPresentation.h>
#include <TemplateRuntimeCapabilityAgent/TemplateRuntime.h>

#include <SmartScreenClient/SmartScreenClient.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/AudioPlayerInfo.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>
#include <SmartScreenSDKInterfaces/GUIClientInterface.h>
#include <SmartScreenSDKInterfaces/GUIServerInterface.h>
#include <SmartScreenSDKInterfaces/NavigationEvent.h>
#include <SmartScreenSDKInterfaces/TemplateRuntimeObserverInterface.h>

#ifdef ENABLE_PCC
#include "PhoneCaller.h"
#endif

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace gui {

/**
 * Manages all GUI related operations to be called from the SDK and the GUIClient
 */
class GUIManager
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::TemplateRuntimeObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    /**
     * Create a GUIManager.
     *
     * @param guiClient A pointer to a GUI Client.
     * @param phoneCaller A pointer to the phone caller if enabled.
     * @param holdToTalkAudioProvider A pointer to a hold to talk audio provider.
     * @param tapToTalkAudioProvider A pointer to a tap to talk audio provider.
     * @param micWrapper A pointer to a microphone wrapper.
     * @param wakeWorkAudioProvider A pointer to an audio wake word provider. This is optional.
     * @param callManager A pointer to a call manager. This is optional.
     * @return An instance of GUIManager.
     */
    static std::shared_ptr<GUIManager> create(
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIClientInterface> guiClient,
#ifdef ENABLE_PCC
        std::shared_ptr<sampleApp::PhoneCaller> phoneCaller,
#endif
        alexaClientSDK::capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
        alexaClientSDK::capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
        std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
        alexaClientSDK::capabilityAgents::aip::AudioProvider wakeWorkAudioProvider =
            alexaClientSDK::capabilityAgents::aip::AudioProvider::null(),
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface> callManager = nullptr);

    /// @name TemplateRuntimeObserverInterface Functions
    /// @{
    void renderTemplateCard(const std::string& jsonPayload, alexaClientSDK::avsCommon::avs::FocusState focusState)
        override;

    void clearTemplateCard() override;

    void renderPlayerInfoCard(
        const std::string& jsonPayload,
        smartScreenSDKInterfaces::AudioPlayerInfo info,
        alexaClientSDK::avsCommon::avs::FocusState focusState) override;

    void clearPlayerInfoCard() override;
    /// @}

    /// @name AlexaPresentationObserverInterface Functions
    /// @{
    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) override;

    void clearDocument() override;

    void executeCommands(const std::string& jsonPayload, const std::string& token) override;

    void interruptCommandSequence() override;
    /// @}

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

    /// @name VisualContextProvider interface Methods
    /// @{
    void provideState(const unsigned int stateRequestToken) override;
    /// }

    /// @name GUIServerInterface methods
    /// @{
    void handleTapToTalk() override;

    void handleHoldToTalk() override;
    /// }

    /// @name AudioPlayerObserverInterface methods
    /// @{
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    /// }

    /**
     * Toggles the microphone state if the Sample App was built with wake word. When the microphone is turned off, the
     * app enters a privacy mode in which it stops recording audio data from the microphone, thus disabling Alexa waking
     * up due to wake word. Note however that hold-to-talk and tap-to-talk modes will still work by recording
     * microphone data temporarily until a user initiated interaction is complete. If this app was built without wake
     * word then this will do nothing as the microphone is already off.
     */
    void handleMicrophoneToggle() override;

    void handleUserEvent(std::string userEventPayload) override;

    void handleVisualContext(uint64_t token, std::string payload) override;

    bool handleFocusAcquireRequest(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) override;

    bool handleFocusReleaseRequest(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) override;

    void handleRenderDocumentResult(std::string token, bool result, std::string error) override;

    void handleExecuteCommandsResult(std::string token, bool result, std::string error) override;

    void handleActivityEvent(
        const std::string& source,
        alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event) override;

    void handleActivityEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event) override;

    void handleNavigationEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::NavigationEvent event) override;

    void setDocumentIdleTimeout(std::chrono::milliseconds timeout) override;

    void handleDeviceWindowState(std::string payload) override;
    /// }

    /**
     * UXDialogObserverInterface methods
     */
    void onDialogUXStateChanged(DialogUXState newState) override;

    /**
     * Set smart screen client
     * @param client The input smart screen client
     */
    void setClient(std::shared_ptr<smartScreenClient::SmartScreenClient> client);

private:
    /**
     * Constructor.
     *
     * @param guiClient A pointer to a GUI Client.
     * @param micWrapper A pointer to a microphone wrapper.
     * @param phoneCaller A pointer to the phone caller if enabled.
     * @param tapToTalkAudioProvider A pointer to a tap to talk audio provider.
     * @param holdToTalkAudioProvider A pointer to a hold to talk audio provider.
     * @param micWrapper A pointer to a microphone wrapper.
     * @param wakeWordAudioProvider A pointer to a wake word audio provider.
     * @param espProvider A pointer to an esp provider. This is optional.
     * @param espModifier A pointer to an esp modifier. This is optional.
     * @param callManager A pointer to a call manager. This is optional.
     */
    GUIManager(
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIClientInterface> guiClient,
#ifdef ENABLE_PCC
        std::shared_ptr<sampleApp::PhoneCaller> phoneCaller,
#endif
        alexaClientSDK::capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
        alexaClientSDK::capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
        std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
        alexaClientSDK::capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface> callManager = nullptr);

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * An internal function to stop the foreground activity
     */
    void executeStopForegroundActivity();

    /**
     * Send UserEvent to AVS.
     *
     * @param payload - The payload for the UserEvent event as defined by AVS.
     */
    void sendUserEvent(const std::string& payload);

    /**
     * Should be called whenever a user presses 'PLAY' for playback.
     */
    void playbackPlay();

    /**
     * Should be called whenever a user presses 'PAUSE' for playback.
     */
    void playbackPause();

    /**
     * Should be called whenever a user presses 'NEXT' for playback.
     */
    void playbackNext();

    /**
     * Should be called whenever a user presses 'PREVIOUS' for playback.
     */
    void playbackPrevious();

    /**
     * Should be called whenever a user presses 'SKIP_FORWARD' for playback.
     */
    void playbackSkipForward();

    /**
     * Should be called whenever a user presses 'SKIP_BACKWARD' for playback.
     */
    void playbackSkipBackward();

    /**
     * sends Gui Toggle event
     * @param toggleType The toggle button for which we send the event.
     * @param Action Does this operation toggles on or off
     */
    void sendGuiToggleEvent(alexaClientSDK::avsCommon::avs::PlaybackToggle toggleType, const bool action);

    /**
     * Should be called when setting value is selected by the user.
     */
    void changeSetting(const std::string& key, const std::string& value);

    /**
     * Should be called whenever a user presses 'SHUFFLE' for playback.
     */
    void playbackShuffle();

    /**
     * Should be called whenever a user presses 'LOOP' for playback.
     */
    void playbackLoop();

    /**
     * Should be called whenever a user presses 'REPEAT' for playback.
     */
    void playbackRepeat();

    /**
     * Should be called whenever a user presses 'THUMBS_UP' for playback.
     */
    void playbackThumbsUp();

    /**
     * Should be called whenever a user presses 'THUMBS_DOWN' for playback.
     */
    void playbackThumbsDown();

    /**
     * Update the firmware version.
     *
     * @param firmwareVersion The new firmware version.
     */
    void setFirmwareVersion(alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion);

    /**
     * Should be called after a user wishes to modify the volume.
     */
    void adjustVolume(alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type type, int8_t delta);

    /**
     * Should be called after a user wishes to set mute.
     */
    void setMute(alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type type, bool mute);

    /**
     * Reset the device and remove any customer data.
     */
    void resetDevice();

    /**
     * Should be called when the user wants to accept a call.
     */
    void acceptCall();

    /**
     * Should be called when the user wants to stop a call.
     */
    void stopCall();

#ifdef ENABLE_PCC
    /**
     * PhoneCallController commands
     */
    void sendCallActivated(const std::string& callId);
    void sendCallTerminated(const std::string& callId);
    void sendCallFailed(const std::string& callId);
    void sendCallReceived(const std::string& callId, const std::string& callerId);
    void sendCallerIdReceived(const std::string& callId, const std::string& callerId);
    void sendInboundRingingStarted(const std::string& callId);
    void sendOutboundCallRequested(const std::string& callId);
    void sendOutboundRingingStarted(const std::string& callId);
    void sendSendDtmfSucceeded(const std::string& callId);
    void sendSendDtmfFailed(const std::string& callId);
#endif

    /**
     * A reference to the audio focus manager
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_audioFocusManager;

    /**
     * A reference to the GUI Client.
     */
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIClientInterface> m_guiClient;

    /**
     * A reference to the smart screen client.
     */
    std::shared_ptr<alexaSmartScreenSDK::smartScreenClient::SmartScreenClient> m_ssClient;

    /**
     * An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
     */
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// The call manager.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface> m_callManager;

#ifdef ENABLE_PCC
    /// The Phone Caller
    std::shared_ptr<sampleApp::PhoneCaller> m_phoneCaller;
#endif

    /// The hold to talk audio provider.
    alexaClientSDK::capabilityAgents::aip::AudioProvider m_holdToTalkAudioProvider;

    /// The tap to talk audio provider.
    alexaClientSDK::capabilityAgents::aip::AudioProvider m_tapToTalkAudioProvider;

    /// The wake word audio provider.
    alexaClientSDK::capabilityAgents::aip::AudioProvider m_wakeWordAudioProvider;

    /// Whether a hold is currently occurring.
    bool m_isHoldOccurring;

    /// Whether a tap is currently occurring.
    bool m_isTapOccurring;

    /// Whether Alexa is speaking or listening.
    bool m_isSpeakingOrListening;

    /// Whether the microphone is currently turned on.
    bool m_isMicOn;

    /// The microphone managing object.
    std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> m_micWrapper;

    /// The currently active smartScreenSDKInterfaces::NonPlayerInfoDisplayType
    smartScreenSDKInterfaces::NonPlayerInfoDisplayType m_activeNonPlayerInfoDisplayType;

    /// The @c PlayerActivity of the @c AudioPlayer
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerActivityState;
    /// @}
};

}  // namespace gui
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_