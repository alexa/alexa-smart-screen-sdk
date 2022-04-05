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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_

#include <Audio/MicrophoneInterface.h>
#include <AFML/FocusManager.h>
#include <AVSCommon/AVS/FocusState.h>
#include <acsdkAudioPlayerInterfaces/AudioPlayerObserverInterface.h>
#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CallStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DtmfObserverInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerObserverInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>

#include <AlexaPresentation/AlexaPresentation.h>
#include <TemplateRuntimeCapabilityAgent/TemplateRuntime.h>

#ifdef ENABLE_RTCSC
#include <SmartScreenSDKInterfaces/LiveViewControllerCapabilityAgentObserverInterface.h>
#endif

#include <SmartScreenClient/SmartScreenClient.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/AudioPlayerInfo.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>
#include <SmartScreenSDKInterfaces/GUIClientInterface.h>
#include <SmartScreenSDKInterfaces/GUIServerInterface.h>
#include <SmartScreenSDKInterfaces/NavigationEvent.h>
#include <SmartScreenSDKInterfaces/TemplateRuntimeObserverInterface.h>
#include <Settings/SettingCallbacks.h>
#include <SampleApp/DoNotDisturbSettingObserver.h>
#include <Settings/DeviceSettingsManager.h>

#ifdef ENABLE_PCC
#include "PhoneCaller.h"
#endif

#ifdef UWP_BUILD
#include <SSSDKCommon/NullMicrophone.h>
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
        , public alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DtmfObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::TemplateRuntimeObserverInterface
        , public alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerObserverInterface
#ifdef ENABLE_RTCSC
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface
#endif
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
    void renderTemplateCard(
        const std::string& token,
        const std::string& jsonPayload,
        alexaClientSDK::avsCommon::avs::FocusState focusState) override;

    void clearTemplateCard(const std::string& token) override;

    void renderPlayerInfoCard(
        const std::string& token,
        const std::string& jsonPayload,
        smartScreenSDKInterfaces::AudioPlayerInfo info,
        alexaClientSDK::avsCommon::avs::FocusState focusState,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> mediaProperties) override;

    void clearPlayerInfoCard(const std::string& token) override;
    /// @}

    /// @name AlexaPresentationObserverInterface Functions
    /// @{

    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) override;

    void clearDocument(const std::string& token) override;

    void executeCommands(const std::string& jsonPayload, const std::string& token) override;

    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token)
        override;

    void interruptCommandSequence(const std::string& token) override;

    void onPresentationSessionChanged(
        const std::string& id,
        const std::string& skillId,
        const std::vector<smartScreenSDKInterfaces::GrantedExtension>& grantedExtensions,
        const std::vector<smartScreenSDKInterfaces::AutoInitializedExtension>& autoInitializedExtensions) override;

    void onRenderDirectiveReceived(const std::string& token, const std::chrono::steady_clock::time_point& receiveTime)
        override;

    void onRenderingAborted(const std::string& token) override;

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
        alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error newError,
        const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>&
            addedOrUpdatedEndpoints,
        const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& deletedEndpoints)
        override;
    /// }

    /// @name VisualStateProviderInterface Methods
    /// @{
    void provideState(const std::string& aplToken, const unsigned int stateRequestToken) override;
    /// }

    /// @name GUIServerInterface methods
    /// @{
    void handleTapToTalk() override;

    void handleHoldToTalk(bool start) override;

    void handleMicrophoneToggle() override;

    void handlePlaybackPlay() override;

    void handlePlaybackPause() override;

    void handlePlaybackNext() override;

    void handlePlaybackPrevious() override;

    void handlePlaybackSkipForward() override;

    void handlePlaybackSkipBackward() override;

    void handlePlaybackToggle(const std::string& name, bool checked) override;

    void handleUserEvent(const std::string& token, std::string userEventPayload) override;

    void onUserEvent() override;

    void handleVisualContext(const std::string& token, uint64_t stateRequestToken, std::string payload) override;

    void handleDataSourceFetchRequestEvent(const std::string& token, std::string type, std::string payload) override;

    void handleRuntimeErrorEvent(const std::string& token, std::string payload) override;

    bool handleFocusAcquireRequest(
        std::string avsInterface,
        std::string channelName,
        alexaClientSDK::avsCommon::avs::ContentType contentType,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) override;

    bool handleFocusReleaseRequest(
        std::string avsInterface,
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) override;

    void handleRenderDocumentResult(std::string token, bool result, std::string error) override;

    void handleExecuteCommandsResult(const std::string& token, const std::string& event, const std::string& message)
        override;

    void handleActivityEvent(
        alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event,
        const std::string& source = "") override;

    void handleNavigationEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::NavigationEvent event) override;

    void setDocumentIdleTimeout(const std::string& token, std::chrono::milliseconds timeout) override;

    void handleDeviceWindowState(std::string payload) override;

    void forceExit() override;

    void handleRenderComplete() override;

    void handleAPLEvent(APLClient::AplRenderingEvent event) override;

    void handleToggleDoNotDisturbEvent() override;

    std::chrono::milliseconds getDeviceTimezoneOffset() override;

    std::chrono::milliseconds getAudioItemOffset() override;

    void handleOnMessagingServerConnectionOpened() override;

    void handleDocumentTerminated(const std::string& token, bool failed) override;

    void acceptCall() override;

    void stopCall() override;

    void enableLocalVideo() override;

    void disableLocalVideo() override;

    void sendDtmf(alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone) override;

    void handleLocaleChange() override;

#ifdef ENABLE_RTCSC
    void handleSetCameraMicrophoneState(bool enabled) override;
    void handleClearLiveView() override;
#endif
    /// }

    /// @name FocusManagerObserverInterface methods
    /// @{
    void onFocusChanged(const std::string& channelName, alexaClientSDK::avsCommon::avs::FocusState newFocus) override;
    /// }

    /// @name AudioPlayerObserverInterface methods
    /// @{
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    /// }

    /**
     * UXDialogObserverInterface methods
     */
    void onDialogUXStateChanged(DialogUXState newState) override;

    /// @name AudioInputProcessorObserverInterface methods.
    /// @{
    void onStateChanged(AudioInputProcessorObserverInterface::State state) override;
    /// @}

    /// @name CallStateObserverInterface methods.
    /// @{
#ifdef ENABLE_COMMS
    void onCallStateInfoChange(
        const alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface::CallStateInfo& newStateInfo)
        override;
#endif
    void onCallStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface::CallState newCallState) override;
    /// @}

    /// @name DtmfObserverInterface methods.
    /// @{
    void onDtmfTonesSent(const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone>&
                             dtmfTones) override;
    /// @}

    /**
     * Set smart screen client
     * @param client The input smart screen client
     */
    void setClient(std::shared_ptr<smartScreenClient::SmartScreenClient> client);

    void onMetricRecorderAvailable(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) override;

    /**
     * Configure settings notifications.
     *
     * @return @true if it succeeds to configure the settings notifications; @c false otherwise.
     */
    bool configureSettingsNotifications();

    /**
     * Set the DoNotDisturbSettingObserver
     *
     * @param Pointer to the observer
     */
    void setDoNotDisturbSettingObserver(std::shared_ptr<sampleApp::DoNotDisturbSettingObserver> doNotDisturbObserver);

#ifdef ENABLE_RTCSC
    /// @name LiveViewControllerCapabilityAgentObserverInterface methods.
    /// @{
    void renderCamera(
        const std::string& payload,
        smartScreenSDKInterfaces::AudioState microphoneAudioState,
        smartScreenSDKInterfaces::ConcurrentTwoWayTalk concurrentTwoWayTalk) override;
    void onCameraStateChanged(smartScreenSDKInterfaces::CameraState cameraState) override;
    void onFirstFrameRendered() override;
    void clearCamera() override;
    /// @}
#endif

#ifdef UWP_BUILD
    void inputAudioFile(const std::string& audioFile);
#endif

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
     * Internal function for handling a @c NavigationEvent::BACK event
     */
    void executeBackNavigation();

    /**
     * Internal function for handling a @c NavigationEvent::EXIT event
     */
    void executeExitNavigation();

    /**
     * Should be called when setting value is selected by the user.
     */
    void changeSetting(const std::string& key, const std::string& value);

    /**
     * Update the firmware version.
     *
     * @param firmwareVersion The new firmware version.
     */
    void setFirmwareVersion(alexaClientSDK::avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion);

    /**
     * Should be called after a user wishes to modify the volume.
     */
    void adjustVolume(alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type, int8_t delta);

    /**
     * Should be called after a user wishes to set mute.
     */
    void setMute(alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type, bool mute);

    /**
     * Handles a change in active @c ASRProfile for the device.
     * @param asrProfile the active ASR profile.
     */
    void handleASRProfileChanged(alexaClientSDK::capabilityAgents::aip::ASRProfile asrProfile);

    /**
     * Reset the device and remove any customer data.
     */
    void resetDevice();

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

    /// Whether a tap is currently occurring.
    bool m_isTapOccurring;

    /// Whether Alexa is speaking or listening.
    bool m_isSpeakingOrListening;

    /// Whether the microphone is currently turned on.
    bool m_isMicOn;

#ifdef ENABLE_RTCSC
    /// State of the active live view camera
    smartScreenSDKInterfaces::CameraState m_cameraState;

    /// State of the active live view camera microphone
    smartScreenSDKInterfaces::AudioState m_cameraMicrophoneAudioState;

    /// Two-Way Talk support for the active live view camera.
    smartScreenSDKInterfaces::ConcurrentTwoWayTalk m_cameraConcurrentTwoWayTalk;
#endif

    /// The microphone managing object.
#ifdef UWP_BUILD
    std::shared_ptr<alexaSmartScreenSDK::sssdkCommon::NullMicrophone> m_micWrapper;
#else
    std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> m_micWrapper;
#endif
    /// The currently active smartScreenSDKInterfaces::NonPlayerInfoDisplayType
    smartScreenSDKInterfaces::NonPlayerInfoDisplayType m_activeNonPlayerInfoDisplayType;

    /// The @c PlayerActivity of the @c AudioPlayer
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerActivityState;

    /// The @c MediaPropertiesInterface for the current @c AudioPlayer
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> m_mediaProperties;

    /// The last state reported by AudioInputProcessor.
    alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State m_audioInputProcessorState;

    /// Map of channel focus states by channelName.
    std::unordered_map<std::string, alexaClientSDK::avsCommon::avs::FocusState> m_channelFocusStates;

    /// Utility flag used for clearing Alert Channel when Foregrounded.
    bool m_clearAlertChannelOnForegrounded;

    /// Object that manages settings callbacks.
    std::shared_ptr<alexaClientSDK::settings::SettingCallbacks<alexaClientSDK::settings::DeviceSettingsManager>>
        m_callbacks;

    /// Object that manages settings.
    std::shared_ptr<alexaClientSDK::settings::DeviceSettingsManager> m_settingsManager;

    /// Observer for DoNotDisturb Setting.
    std::shared_ptr<sampleApp::DoNotDisturbSettingObserver> m_doNotDisturbObserver;

    /// The interface holding audio focus.
    std::string m_interfaceHoldingAudioFocus;

    /// The active ASR profile.
    alexaClientSDK::capabilityAgents::aip::ASRProfile m_asrProfile;
};

}  // namespace gui
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_GUI_INCLUDE_SAMPLEAPP_GUI_GUIMANAGER_H_
