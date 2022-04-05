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

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <AVSCommon/AVS/PlaybackButtons.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <Settings/SettingObserverInterface.h>
#include <SampleApp/Messages/GUIClientMessage.h>

#include "SampleApp/GUI/GUIManager.h"

#ifdef UWP_BUILD
#include <UWPSampleApp/Utils.h>
#include <SSSDKCommon/AudioFileUtil.h>
#endif

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace gui {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::avs;

using namespace smartScreenSDKInterfaces;

/// String to identify log entries originating from this file.
static const std::string TAG("GUIManager");

/**
 * Create a LogEntry using this file's TAG and the specified event string.x1
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// Interface name to use for focus requests.
static const std::string APL_INTERFACE("Alexa.Presentation.APL");

/// String to identify the Shuffle Toggle of PlaybackController.
static const std::string SHUFFLE_TOGGLE_ID("shuffle");

/// String to identify the Loop Toggle of PlaybackController.
static const std::string LOOP_TOGGLE_ID("loop");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string REPEAT_TOGGLE_ID("repeat");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string THUMBSUP_TOGGLE_ID("thumbsUp");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string THUMBSDOWN_TOGGLE_ID("thumbsDown");

/// The name of the do not disturb confirmation setting.
static const std::string DO_NOT_DISTURB_NAME = "DoNotDisturb";

/// Map to match a toggle command id to the corresponding enum value.
static const std::map<std::string, avsCommon::avs::PlaybackToggle> TOGGLE_COMMAND_ID_TO_TOGGLE = {
    {SHUFFLE_TOGGLE_ID, avsCommon::avs::PlaybackToggle::SHUFFLE},
    {LOOP_TOGGLE_ID, avsCommon::avs::PlaybackToggle::LOOP},
    {REPEAT_TOGGLE_ID, avsCommon::avs::PlaybackToggle::LOOP},
    {THUMBSUP_TOGGLE_ID, avsCommon::avs::PlaybackToggle::THUMBS_UP},
    {THUMBSDOWN_TOGGLE_ID, avsCommon::avs::PlaybackToggle::THUMBS_DOWN}};

std::shared_ptr<GUIManager> GUIManager::create(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIClientInterface> guiClient,
#ifdef ENABLE_PCC
    m_phoneCaller{phoneCaller},
#endif
    alexaClientSDK::capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
    alexaClientSDK::capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
    std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
    alexaClientSDK::capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface> callManager) {
    if (!guiClient) {
        ACSDK_CRITICAL(LX(__func__).d("reason", "null guiClient"));
        return nullptr;
    }
    if (!holdToTalkAudioProvider) {
        ACSDK_CRITICAL(LX(__func__).d("reason", "null holdToTalkAudioProvider"));
        return nullptr;
    }
    if (!tapToTalkAudioProvider) {
        ACSDK_CRITICAL(LX(__func__).d("reason", "null tapToTalkAudioProvider"));
        return nullptr;
    }
    if (!micWrapper) {
        ACSDK_CRITICAL(LX(__func__).d("reason", "null micWrapper"));
        return nullptr;
    }

    return std::shared_ptr<GUIManager>(new GUIManager(
        guiClient, holdToTalkAudioProvider, tapToTalkAudioProvider, micWrapper, wakeWordAudioProvider, callManager));
}

GUIManager::GUIManager(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIClientInterface> guiClient,
#ifdef ENABLE_PCC
    m_phoneCaller{phoneCaller},
#endif
    alexaClientSDK::capabilityAgents::aip::AudioProvider holdToTalkAudioProvider,
    alexaClientSDK::capabilityAgents::aip::AudioProvider tapToTalkAudioProvider,
    std::shared_ptr<alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface> micWrapper,
    alexaClientSDK::capabilityAgents::aip::AudioProvider wakeWordAudioProvider,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface> callManager) :
        RequiresShutdown{"GUIManager"},
        m_callManager{callManager},
        m_holdToTalkAudioProvider{holdToTalkAudioProvider},
        m_tapToTalkAudioProvider{tapToTalkAudioProvider},
        m_wakeWordAudioProvider{wakeWordAudioProvider},
        m_activeNonPlayerInfoDisplayType{NonPlayerInfoDisplayType::NONE},
        m_playerActivityState{PlayerActivity::FINISHED} {
    m_guiClient = guiClient;
    m_isMicOn = true;
    m_isTapOccurring = false;
    m_isSpeakingOrListening = false;
    m_clearAlertChannelOnForegrounded = false;
    m_audioInputProcessorState = AudioInputProcessorObserverInterface::State::IDLE;
    m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_NAME] = FocusState::NONE;
    m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::ALERT_CHANNEL_NAME] = FocusState::NONE;
    m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::CONTENT_CHANNEL_NAME] = FocusState::NONE;
    m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::COMMUNICATIONS_CHANNEL_NAME] =
        FocusState::NONE;
    m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_NAME] = FocusState::NONE;
    m_interfaceHoldingAudioFocus = {};
    m_asrProfile = alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD;
#ifdef ENABLE_RTCSC
    m_cameraState = smartScreenSDKInterfaces::CameraState::UNKNOWN;
    m_cameraMicrophoneAudioState = smartScreenSDKInterfaces::AudioState::UNKNOWN;
    m_cameraConcurrentTwoWayTalk = smartScreenSDKInterfaces::ConcurrentTwoWayTalk::UNKNOWN;
#endif

#ifdef UWP_BUILD
    m_micWrapper = std::dynamic_pointer_cast<alexaSmartScreenSDK::sssdkCommon::NullMicrophone>(micWrapper);
#else
    m_micWrapper = micWrapper;
#endif

    m_micWrapper->startStreamingMicrophoneData();

    if (m_wakeWordAudioProvider) {
        handleASRProfileChanged(m_wakeWordAudioProvider.profile);
    }
}

void GUIManager::renderTemplateCard(const std::string& token, const std::string& jsonPayload, FocusState focusState) {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::RENDER_TEMPLATE;
    m_guiClient->renderTemplateCard(token, jsonPayload, focusState);
}

void GUIManager::clearTemplateCard(const std::string& token) {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::NONE;
    m_guiClient->clearTemplateCard(token);
}

void GUIManager::renderPlayerInfoCard(
    const std::string& token,
    const std::string& jsonPayload,
    smartScreenSDKInterfaces::AudioPlayerInfo info,
    FocusState focusState,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> mediaProperties) {
    m_mediaProperties = mediaProperties;
    m_guiClient->renderPlayerInfoCard(token, jsonPayload, info, focusState, mediaProperties);
}

void GUIManager::clearPlayerInfoCard(const std::string& token) {
    m_guiClient->clearPlayerInfoCard(token);
}

void GUIManager::interruptCommandSequence(const std::string& token) {
    m_guiClient->interruptCommandSequence(token);
}

void GUIManager::onPresentationSessionChanged(
    const std::string& id,
    const std::string& skillId,
    const std::vector<smartScreenSDKInterfaces::GrantedExtension>& grantedExtensions,
    const std::vector<smartScreenSDKInterfaces::AutoInitializedExtension>& autoInitializedExtensions) {
    m_guiClient->onPresentationSessionChanged(id, skillId, grantedExtensions, autoInitializedExtensions);
}

void GUIManager::renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::ALEXA_PRESENTATION;
    m_guiClient->renderDocument(jsonPayload, token, windowId);
}

void GUIManager::clearDocument(const std::string& token) {
    ACSDK_DEBUG5(LX(__func__).d("token", token));
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::NONE;
    m_guiClient->clearDocument(token);
}

void GUIManager::executeCommands(const std::string& jsonPayload, const std::string& token) {
    m_guiClient->executeCommands(jsonPayload, token);
}

void GUIManager::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    m_guiClient->dataSourceUpdate(sourceType, jsonPayload, token);
}

void GUIManager::handleTapToTalk() {
    ACSDK_DEBUG9(LX("handleTapToTalk"));
    m_executor
        .submit([this]() {
            handleASRProfileChanged(m_tapToTalkAudioProvider.profile);
            if (!m_isMicOn) {
                return;
            }
            if (!m_isTapOccurring) {
                if (m_ssClient->notifyOfTapToTalk(m_tapToTalkAudioProvider).get()) {
                    m_isTapOccurring = true;
                }
            } else {
                m_isTapOccurring = false;
                m_ssClient->notifyOfTapToTalkEnd();
            }
        })
#ifdef UWP_BUILD
        .wait()
#endif
        ;
}

void GUIManager::handleHoldToTalk(bool start) {
    ACSDK_DEBUG9(LX(__func__).d("start", start));
    m_executor.submit([this, start]() {
        handleASRProfileChanged(m_holdToTalkAudioProvider.profile);
        if (!m_isMicOn) {
            return;
        }
        bool activeCameraWithMicrophone = false;

        // Set the value of @c isHoldOccurring to what UI thinks it should be.
        // There could be circumstances where the user applications might fall out
        // of sync with SDK about the status of hold-to-talk.
        bool isHoldOccurring = !start;

#ifdef ENABLE_RTCSC
        // Mic input is fully routed to active camera and not Alexa AIP when:
        // - ASR Profile is CLOSE_TALK (physical remote mic input)
        // - Camera is displayed
        // - Camera is CONNECTED
        // - Camera supports microphone
        // - Camera supports Two-Way talk (concurrent or not)
        activeCameraWithMicrophone =
            (alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK == m_asrProfile &&
             NonPlayerInfoDisplayType::LIVE_VIEW == m_activeNonPlayerInfoDisplayType &&
             smartScreenSDKInterfaces::CameraState::CONNECTED == m_cameraState &&
             (smartScreenSDKInterfaces::AudioState::DISABLED != m_cameraMicrophoneAudioState &&
              smartScreenSDKInterfaces::AudioState::UNKNOWN != m_cameraMicrophoneAudioState) &&
             smartScreenSDKInterfaces::ConcurrentTwoWayTalk::UNKNOWN != m_cameraConcurrentTwoWayTalk);
#endif
        if (!isHoldOccurring) {
            // If we have no active 2-way talk camera, route mic input to Alexa AIP provider as usual.
            isHoldOccurring =
                activeCameraWithMicrophone || m_ssClient->notifyOfHoldToTalkStart(m_holdToTalkAudioProvider).get();
        } else {
            isHoldOccurring = false;
            if (!activeCameraWithMicrophone) {
                m_ssClient->notifyOfHoldToTalkEnd();
            }
        }

#ifdef ENABLE_RTCSC
        // If we have an active 2-way camera, enable/disable its microphone.
        if (activeCameraWithMicrophone) {
            // Set camera mic state
            handleSetCameraMicrophoneState(isHoldOccurring);
            // Inform GUI of camera mic state
            m_guiClient->handleCameraMicrophoneStateChanged(isHoldOccurring);
        }
#endif
    });
}

void GUIManager::handleMicrophoneToggle() {
    ACSDK_DEBUG5(LX(__func__));
    m_executor.submit([this]() {
        if (!m_wakeWordAudioProvider) {
            return;
        }
        if (m_isMicOn) {
            m_isMicOn = false;
            m_micWrapper->stopStreamingMicrophoneData();
        } else {
            m_isMicOn = true;
            m_micWrapper->startStreamingMicrophoneData();
        }
    });
}

void GUIManager::handleUserEvent(const std::string& token, std::string userEventPayload) {
    m_executor.submit([this, userEventPayload]() { m_ssClient->sendUserEvent(userEventPayload); });
}

void GUIManager::handleDataSourceFetchRequestEvent(const std::string& token, std::string type, std::string payload) {
    m_executor.submit([this, type, payload] { m_ssClient->sendDataSourceFetchRequestEvent(type, payload); });
}

void GUIManager::handleRuntimeErrorEvent(const std::string& token, std::string payload) {
    m_executor.submit([this, token, payload] { m_ssClient->sendRuntimeErrorEvent(payload); });
}

void GUIManager::handlePlaybackPlay() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PLAY); });
}

void GUIManager::handlePlaybackPause() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PAUSE); });
}

void GUIManager::handlePlaybackNext() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::NEXT); });
}

void GUIManager::handlePlaybackPrevious() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PREVIOUS); });
}

void GUIManager::handlePlaybackSkipForward() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::SKIP_FORWARD); });
}

void GUIManager::handlePlaybackSkipBackward() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::SKIP_BACKWARD); });
}

void GUIManager::handlePlaybackToggle(const std::string& name, bool checked) {
    m_executor.submit([this, name, checked]() {
        auto it = TOGGLE_COMMAND_ID_TO_TOGGLE.find(name);
        if (it == TOGGLE_COMMAND_ID_TO_TOGGLE.end()) {
            ACSDK_ERROR(LX(__func__).d("Invalid Toggle Name", name));
            return;
        }
        m_ssClient->getPlaybackRouter()->togglePressed(it->second, checked);
    });
}

void GUIManager::setFirmwareVersion(avsCommon::sdkInterfaces::softwareInfo::FirmwareVersion firmwareVersion) {
    m_executor.submit([this, firmwareVersion]() { m_ssClient->setFirmwareVersion(firmwareVersion); });
}

void GUIManager::adjustVolume(avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type, int8_t delta) {
    m_executor.submit([this, type, delta]() {
        /*
         * Group the unmute action as part of the same affordance that caused the volume change, so we don't
         * send another event. This isn't a requirement by AVS.
         */
        std::future<bool> unmuteFuture = m_ssClient->getSpeakerManager()->setMute(type, false, true);
        if (!unmuteFuture.valid()) {
            return;
        }
        unmuteFuture.get();

        std::future<bool> future = m_ssClient->getSpeakerManager()->adjustVolume(type, delta);
        if (!future.valid()) {
            return;
        }
        future.get();
    });
}

void GUIManager::setMute(avsCommon::sdkInterfaces::ChannelVolumeInterface::Type type, bool mute) {
    m_executor.submit([this, type, mute]() {
        std::future<bool> future = m_ssClient->getSpeakerManager()->setMute(type, mute);
        if (!future.valid()) {
            return;
        }
        future.get();
    });
}

void GUIManager::handleASRProfileChanged(alexaClientSDK::capabilityAgents::aip::ASRProfile asrProfile) {
    if (asrProfile != m_asrProfile) {
        m_asrProfile = asrProfile;
        m_guiClient->handleASRProfileChanged(m_asrProfile);
    }
}

void GUIManager::resetDevice() {
    // This is a blocking operation. No interaction will be allowed during / after resetDevice
    auto result = m_executor.submit([this]() { m_ssClient->getRegistrationManager()->logout(); });
    result.wait();
}

void GUIManager::acceptCall() {
    m_executor.submit([this]() {
        if (m_ssClient->isCommsEnabled()) {
            m_ssClient->acceptCommsCall();
        } else {
            ACSDK_WARN(LX(__func__).m("Communication not supported."));
        }
    });
}

void GUIManager::stopCall() {
    m_executor.submit([this]() {
        if (m_ssClient->isCommsEnabled()) {
            m_ssClient->stopCommsCall();
        } else {
            ACSDK_WARN(LX(__func__).m("Communication not supported."));
        }
    });
}

void GUIManager::enableLocalVideo() {
    m_executor.submit([this]() {
        if (m_ssClient->isCommsEnabled()) {
            m_ssClient->enableLocalVideo();
        } else {
            ACSDK_WARN(LX(__func__).m("Communication not supported."));
        }
    });
}

void GUIManager::disableLocalVideo() {
    m_executor.submit([this]() {
        if (m_ssClient->isCommsEnabled()) {
            m_ssClient->disableLocalVideo();
        } else {
            ACSDK_WARN(LX(__func__).m("Communication not supported."));
        }
    });
}

void GUIManager::sendDtmf(alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone) {
    m_executor.submit([this, dtmfTone]() {
        if (m_ssClient->isCommsEnabled()) {
            m_ssClient->sendDtmf(dtmfTone);
        } else {
            ACSDK_WARN(LX(__func__).m("Communication not supported."));
        }
    });
}

#ifdef ENABLE_PCC
void GUIManager::sendCallActivated(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendCallActivated(callId);
        }
    });
}
void GUIManager::sendCallTerminated(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendCallTerminated(callId);
        }
    });
}

void GUIManager::sendCallFailed(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendCallFailed(callId);
        }
    });
}

void GUIManager::sendCallReceived(const std::string& callId, const std::string& callerId) {
    m_executor.submit([this, callId, callerId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendCallReceived(callId, callerId);
        }
    });
}

void GUIManager::sendCallerIdReceived(const std::string& callId, const std::string& callerId) {
    m_executor.submit([this, callId, callerId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendCallerIdReceived(callId, callerId);
        }
    });
}

void GUIManager::sendInboundRingingStarted(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendInboundRingingStarted(callId);
        }
    });
}

void GUIManager::sendOutboundCallRequested(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendDialStarted(callId);
        }
    });
}

void GUIManager::sendOutboundRingingStarted(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendOutboundRingingStarted(callId);
        }
    });
}

void GUIManager::sendSendDtmfSucceeded(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendSendDtmfSucceeded(callId);
        }
    });
}

void GUIManager::sendSendDtmfFailed(const std::string& callId) {
    m_executor.submit([this, callId]() {
        if (m_phoneCaller) {
            m_phoneCaller->sendSendDtmfFailed(callId);
        }
    });
}
#endif

void GUIManager::handleVisualContext(const std::string& token, uint64_t stateRequestToken, std::string payload) {
    m_executor.submit(
        [this, stateRequestToken, payload]() { m_ssClient->handleVisualContext(stateRequestToken, payload); });
}

bool GUIManager::handleFocusAcquireRequest(
    std::string avsInterface,
    std::string channelName,
    alexaClientSDK::avsCommon::avs::ContentType contentType,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_executor
        .submit([this, channelName, channelObserver, contentType, avsInterface]() {
            auto activity = alexaClientSDK::acl::FocusManagerInterface::Activity::create(
                avsInterface, channelObserver, std::chrono::milliseconds::zero(), contentType);
            bool focusAcquired = m_ssClient->getAudioFocusManager()->acquireChannel(channelName, activity);
            if (focusAcquired) {
                m_interfaceHoldingAudioFocus = avsInterface;
            }
            return focusAcquired;
        })
        .get();
}

bool GUIManager::handleFocusReleaseRequest(
    std::string avsInterface,
    std::string channelName,
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_executor
        .submit([this, avsInterface, channelName, channelObserver]() {
            if (avsInterface == m_interfaceHoldingAudioFocus) {
                bool focusReleased =
                    m_ssClient->getAudioFocusManager()->releaseChannel(channelName, channelObserver).get();
                if (focusReleased) {
                    m_interfaceHoldingAudioFocus.clear();
                }
                return focusReleased;
            }
            return false;
        })
        .get();
}

void GUIManager::handleRenderDocumentResult(std::string token, bool result, std::string error) {
    m_executor.submit([this, result, token, error]() { m_ssClient->handleRenderDocumentResult(token, result, error); });
}

void GUIManager::handleExecuteCommandsResult(
    const std::string& token,
    const std::string& event,
    const std::string& message) {
    m_executor.submit(
        [this, token, event, message]() { m_ssClient->handleExecuteCommandsResult(token, event, message); });
}

void GUIManager::handleActivityEvent(smartScreenSDKInterfaces::ActivityEvent event, const std::string& source) {
    m_executor.submit([this, source, event]() {
        if (smartScreenSDKInterfaces::ActivityEvent::INTERRUPT == event && m_isSpeakingOrListening) {
            ACSDK_DEBUG3(LX(__func__).d(
                "Interrupted activity while speaking or listening",
                smartScreenSDKInterfaces::activityEventToString(event)));
            m_ssClient->releaseAllObserversOnDialogChannel();
            m_ssClient->clearActiveExecuteCommandsDirective();
        }
        m_ssClient->handleActivityEvent(
            source.empty() ? TAG : source,
            event,
            NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
    });
}

void GUIManager::handleNavigationEvent(smartScreenSDKInterfaces::NavigationEvent event) {
    m_executor.submit([this, event]() {
        ACSDK_DEBUG3(LX(__func__).d(
            "processNavigationEvent in executor", smartScreenSDKInterfaces::navigationEventToString(event)));

        // TODO : Implement more robust visual presentation and granular channel orchestration for local navigation
        switch (event) {
            case smartScreenSDKInterfaces::NavigationEvent::BACK:
                executeBackNavigation();
                break;
            case smartScreenSDKInterfaces::NavigationEvent::EXIT:
                executeExitNavigation();
                break;
            default:
                // Not possible as returned above.
                break;
        }
    });
}

void GUIManager::executeBackNavigation() {
    /**
     * Back Navigation supports the following use cases:
     * 1. GUIClient managed back, for traversal of a UI client implemented backstack.
     * 2. Back from ALL other active audio channel(s) and /or visual card to audio content/PlayerInfo card.
     * 3. Back from alert/dialog audio channels to active live view camera.
     * 4. Back from audio content content/PlayerInfo card to 'home' state.
     */

    bool dialogChannelActive =
        FocusState::NONE != m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::DIALOG_CHANNEL_NAME];
    bool alertChannelActive =
        FocusState::NONE != m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::ALERT_CHANNEL_NAME];
    bool contentChannelActive =
        FocusState::NONE != m_channelFocusStates[avsCommon::sdkInterfaces::FocusManagerInterface::CONTENT_CHANNEL_NAME];
    bool nonPlayerInfoActive = NonPlayerInfoDisplayType::NONE != m_activeNonPlayerInfoDisplayType;

    /**
     * Always stop the foreground activity unless:
     * - playing audio content,
     * - AND dialog and alerts channels are NOT active,
     * - AND displaying presentation over PlayerInfo.
     * In that case back will handle clearing the presentation over PlayerInfo, but will persist active audio content.
     */
    bool stopForegroundActivity = true;
    if (PlayerActivity::PLAYING == m_playerActivityState && !dialogChannelActive && !alertChannelActive &&
        NonPlayerInfoDisplayType::NONE != m_activeNonPlayerInfoDisplayType) {
        stopForegroundActivity = false;
    }

    /**
     * Always clear the displayed presentations unless:
     * - dialog OR alerts channel is active
     * - AND audio content channel is active, but there is no NonPlayerInfoDisplay UI displayed,
     * - OR live view camera is active
     * In that case we should stop the foreground activity (the dialog or alert), but not clear the presentation.
     */
    bool clearPresentations = true;
    if ((dialogChannelActive || alertChannelActive) &&
        ((contentChannelActive && !nonPlayerInfoActive) ||
         (NonPlayerInfoDisplayType::LIVE_VIEW == m_activeNonPlayerInfoDisplayType))) {
        clearPresentations = false;
    }

    /**
     * Stopping foreground audio activity happens before we allow GUIClient to handle 'visual' back navigation.
     */
    if (stopForegroundActivity) {
        /**
         * If both dialog and alerts are active,
         * stop dialog first (which has priority), and then stop alerts when it becomes foregrounded.
         */
        if (dialogChannelActive && alertChannelActive) {
            m_clearAlertChannelOnForegrounded = true;
        }
        m_ssClient->stopForegroundActivity();
    }

    /**
     * BACK will attempt to let the GUIClient handle visual navigation before clearing.
     * This allows for things like backstack traversal if implemented by the client.
     */
    if (!m_guiClient->handleNavigationEvent(NavigationEvent::BACK)) {
        /// Clear cloud context unless waiting to clear Alert channel first
        if (!m_clearAlertChannelOnForegrounded) {
            m_ssClient->forceClearDialogChannelFocus();
        }
        if (clearPresentations) {
            /// Always stop active APL commands when clearing presentations.
            m_ssClient->clearActiveExecuteCommandsDirective();
            m_ssClient->clearPresentations();
            /// Always attempt to clear the playerInfo card if there's nothing else displayed and visual focus has been
            /// released
            if (!contentChannelActive || !nonPlayerInfoActive) {
                m_ssClient->clearPlayerInfoCard();
            }
        }
    }
}

void GUIManager::executeExitNavigation() {
    m_ssClient->forceExit();
}

void GUIManager::forceExit() {
    m_executor.submit([this]() { executeExitNavigation(); });
}

void GUIManager::setDocumentIdleTimeout(const std::string& token, std::chrono::milliseconds timeout) {
    m_executor.submit([this, timeout]() { m_ssClient->setDocumentIdleTimeout(timeout); });
}

void GUIManager::handleDeviceWindowState(std::string payload) {
    m_executor.submit([this, payload]() { m_ssClient->setDeviceWindowState(payload); });
}

void GUIManager::handleRenderComplete() {
    m_executor.submit([this]() {
        bool isAlexaPresentationPresenting =
            NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType;
        m_ssClient->handleRenderComplete(isAlexaPresentationPresenting);
    });
}

void GUIManager::handleAPLEvent(APLClient::AplRenderingEvent event) {
    m_ssClient->handleAPLEvent(event, NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
}

void GUIManager::handleToggleDoNotDisturbEvent() {
    m_ssClient->getSettingsManager()->setValue<settings::DO_NOT_DISTURB>(
        !m_ssClient->getSettingsManager()->getValue<settings::DO_NOT_DISTURB>(false).second);
}

void GUIManager::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
}

void GUIManager::onCapabilitiesStateChange(
    avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State newState,
    avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error newError,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& addedOrUpdatedEndpoints,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& deletedEndpoints) {
}

void GUIManager::provideState(const std::string& aplToken, const unsigned int stateRequestToken) {
    m_executor.submit(
        [this, aplToken, stateRequestToken]() { m_guiClient->provideState(aplToken, stateRequestToken); });
}

#ifdef ENABLE_COMMS
void GUIManager::onCallStateInfoChange(const CallStateInfo& stateInfo) {
    // Send call state information to GUI
    m_guiClient->sendCallStateInfo(stateInfo);
}
#endif

void GUIManager::onDtmfTonesSent(
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone>& dtmfTones) {
#ifdef ENABLE_COMMS
    ACSDK_DEBUG(LX("onDtmfTonesSent"));
    m_guiClient->notifyDtmfTonesSent(dtmfTones);
#endif
}

void GUIManager::onCallStateChange(CallState callState) {
}

void GUIManager::onDialogUXStateChanged(DialogUXState state) {
    m_executor.submit([this, state]() {
        switch (state) {
            case DialogUXState::SPEAKING:
                m_isSpeakingOrListening = true;
                m_isTapOccurring = false;
                break;
            case DialogUXState::EXPECTING:
            case DialogUXState::FINISHED:
            case DialogUXState::IDLE:
            case DialogUXState::THINKING:
                m_isTapOccurring = false;
                m_isSpeakingOrListening = false;
                break;
            case DialogUXState::LISTENING:
                m_isSpeakingOrListening = true;
            default:
                break;
        }
    });
}

void GUIManager::onUserEvent() {
    m_ssClient->onUserEvent(m_audioInputProcessorState);
}

void GUIManager::onStateChanged(AudioInputProcessorObserverInterface::State state) {
    m_audioInputProcessorState = state;

    // Interrupt activity on speech recognizing
    if (state == AudioInputProcessorObserverInterface::State::RECOGNIZING) {
        handleActivityEvent(
            smartScreenSDKInterfaces::ActivityEvent::INTERRUPT,
            "AudioInputProcessor" + AudioInputProcessorObserverInterface::stateToString(state));
    }
}

void GUIManager::onPlayerActivityChanged(avsCommon::avs::PlayerActivity state, const Context& context) {
    m_executor.submit([this, state]() { m_playerActivityState = state; });
}

void GUIManager::onFocusChanged(const std::string& channelName, FocusState newFocus) {
    m_executor.submit([this, channelName, newFocus]() {
        ACSDK_DEBUG(
            LX("ChannelFocusChanged").d("channelName", channelName).d("newFocus", focusStateToString(newFocus)));

        m_channelFocusStates[channelName] = newFocus;

        /// Handle use case to clear Alerts channel when foregrounded.
        if (channelName == avsCommon::sdkInterfaces::FocusManagerInterface::ALERT_CHANNEL_NAME &&
            FocusState::FOREGROUND == newFocus && m_clearAlertChannelOnForegrounded) {
            m_ssClient->stopForegroundActivity();
            m_ssClient->forceClearDialogChannelFocus();
            m_clearAlertChannelOnForegrounded = false;
        }

        /// Handle use case to try and force display PlayerInfo if the visual channel is cleared.
        if (channelName == avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_NAME &&
            FocusState::NONE == newFocus) {
            m_ssClient->forceDisplayPlayerInfoCard();
        }
    });
}

void GUIManager::setClient(std::shared_ptr<smartScreenClient::SmartScreenClient> client) {
    auto result = m_executor.submit([this, client]() {
        if (!client) {
            ACSDK_CRITICAL(LX(__func__).d("reason", "null client"));
        }
        m_ssClient = client;
    });
    result.wait();
}

std::chrono::milliseconds GUIManager::getAudioItemOffset() {
    if (!m_mediaProperties) {
        ACSDK_ERROR(LX("getAudioItemOffset").d("reason", "Null MediaPropertiesInterface"));
        return std::chrono::milliseconds::zero();
    }
    return m_mediaProperties->getAudioItemOffset();
}

std::chrono::milliseconds GUIManager::getDeviceTimezoneOffset() {
    return m_ssClient->getDeviceTimezoneOffset();
}

void GUIManager::doShutdown() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.shutdown();
    m_audioFocusManager.reset();
    m_ssClient.reset();
    m_guiClient.reset();
    if (m_callManager) {
        m_callManager->shutdown();
        m_callManager.reset();
    }
    m_micWrapper.reset();
}

void GUIManager::onRenderDirectiveReceived(
    const std::string& token,
    const std::chrono::steady_clock::time_point& receiveTime) {
    m_guiClient->onRenderDirectiveReceived(token, receiveTime);
}

void GUIManager::onRenderingAborted(const std::string& token) {
    m_guiClient->onRenderingAborted(token);
}

void GUIManager::onMetricRecorderAvailable(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    m_guiClient->onMetricRecorderAvailable(metricRecorder);
}

bool GUIManager::configureSettingsNotifications() {
    bool future =
        m_executor
            .submit([this]() {
                m_settingsManager = m_ssClient->getSettingsManager();
                m_callbacks =
                    alexaClientSDK::settings::SettingCallbacks<alexaClientSDK::settings::DeviceSettingsManager>::create(
                        m_settingsManager);

                if (!m_callbacks) {
                    ACSDK_ERROR(LX("configureSettingsNotificationsFailed").d("reason", "createCallbacksFailed"));
                    return false;
                }

                bool callback = m_callbacks->add<alexaClientSDK::settings::DeviceSettingsIndex::DO_NOT_DISTURB>(
                    [this](bool enable, alexaClientSDK::settings::SettingNotifications notifications) {
                        if (m_doNotDisturbObserver && m_settingsManager) {
                            m_doNotDisturbObserver->onDoNotDisturbSettingChanged(
                                m_settingsManager->getValue<settings::DO_NOT_DISTURB>(false).second);
                        }
                    });

                callback &= m_callbacks->add<alexaClientSDK::settings::DeviceSettingsIndex::LOCALE>(
                    [this](
                        const settings::DeviceLocales& value,
                        alexaClientSDK::settings::SettingNotifications notifications) { handleLocaleChange(); });
                return callback;
            })
            .get();

    return future;
}

void GUIManager::handleLocaleChange() {
    auto localeSetting = m_ssClient->getSettingsManager()->getValue<settings::DeviceSettingsIndex::LOCALE>();
    if (!localeSetting.first) {
        ACSDK_WARN(LX(__func__).m("Invalid locales array from settings."));
        return;
    }
    auto locales = localeSetting.second;
    rapidjson::Document document;
    document.SetArray();

    auto& allocator = document.GetAllocator();
    for (const auto& locale : locales) {
        document.PushBack(rapidjson::Value().SetString(locale.c_str(), locale.length(), allocator), allocator);
    }

    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);

    auto localeStr = strbuf.GetString();
    ACSDK_DEBUG3(LX(__func__).d("LocaleChanged", localeStr));
    auto message = messages::LocaleChangeMessage(localeStr);
    m_guiClient->sendMessage(message);
}

void GUIManager::setDoNotDisturbSettingObserver(
    std::shared_ptr<sampleApp::DoNotDisturbSettingObserver> doNotDisturbObserver) {
    m_doNotDisturbObserver = std::move(doNotDisturbObserver);
}

void GUIManager::handleOnMessagingServerConnectionOpened() {
    if (m_doNotDisturbObserver && m_settingsManager) {
        m_doNotDisturbObserver->onDoNotDisturbSettingChanged(
            m_settingsManager->getValue<settings::DO_NOT_DISTURB>(false).second);
    }
}

void GUIManager::handleDocumentTerminated(const std::string& token, bool failed) {
    m_ssClient->clearActiveExecuteCommandsDirective(token, failed);
    m_ssClient->clearAPLCard();
    // Only stop audio if it is coming from APL Audio (SpeakItem, SpeakList, etc.)
    if (APL_INTERFACE == m_interfaceHoldingAudioFocus) {
        m_ssClient->stopForegroundActivity();
    }
}

#if ENABLE_RTCSC
void GUIManager::handleSetCameraMicrophoneState(bool enabled) {
    ACSDK_DEBUG5(LX(__func__));
    m_ssClient->setCameraMicrophoneState(enabled);
}

void GUIManager::handleClearLiveView() {
    m_ssClient->clearLiveView();
}

void GUIManager::renderCamera(
    const std::string& payload,
    smartScreenSDKInterfaces::AudioState microphoneAudioState,
    smartScreenSDKInterfaces::ConcurrentTwoWayTalk concurrentTwoWayTalk) {
    m_cameraMicrophoneAudioState = microphoneAudioState;
    m_cameraConcurrentTwoWayTalk = concurrentTwoWayTalk;
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::LIVE_VIEW;
    m_guiClient->renderCamera(payload, microphoneAudioState, concurrentTwoWayTalk);
    // Enable the camera mic on init if it is UNMUTED and supports TWO_WAY_TALK,
    // AND the device is not using a CLOSE_TALK ASR Profile
    bool micInitEnabled = alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK != m_asrProfile &&
                          smartScreenSDKInterfaces::AudioState::UNMUTED == m_cameraMicrophoneAudioState &&
                          m_cameraConcurrentTwoWayTalk == smartScreenSDKInterfaces::ConcurrentTwoWayTalk::ENABLED;
    m_guiClient->handleCameraMicrophoneStateChanged(micInitEnabled);
}

void GUIManager::onCameraStateChanged(smartScreenSDKInterfaces::CameraState cameraState) {
    m_cameraState = cameraState;
    m_guiClient->onCameraStateChanged(cameraState);
}

void GUIManager::onFirstFrameRendered() {
    m_guiClient->onFirstFrameRendered();
}

void GUIManager::clearCamera() {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::NONE;
    m_guiClient->clearCamera();
}
#endif

#ifdef UWP_BUILD
void GUIManager::inputAudioFile(const std::string& audioFile) {
    bool errorOccured = false;
    auto audioData = alexaSmartScreenSDK::sssdkCommon::AudioFileUtil::readAudioFromFile(audioFile, errorOccured);
    if (errorOccured) {
        return;
    }
    handleTapToTalk();
    m_micWrapper->writeAudioData(audioData);
}
#endif

}  // namespace gui
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
