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

/// String to identify the ARGUMENTS tag
static const std::string ARGUMENTS_TAG("arguments");

/// The command type index in the arguments array.
static const int COMMAND_TYPE_INDEX = 0;

/// The command id index in the arguments array.
static const int COMMAND_ID_INDEX = 1;

/// The action index in the arguments array.
static const int COMMAND_ACTION_INDEX = 2;

/// String to identify the Button command argument.
static const std::string BUTTON_COMMAND_TYPE("ButtonCommandIssued");

/// String to identify the Toggle command argument.
static const std::string TOGGLE_COMMAND_TYPE("ToggleCommandIssued");

/// String to identify the Button command argument.
static const std::string PLAYPAUSE_COMMAND_TYPE("PlayCommandIssued");

/// String to identify the Toggle command argument.
static const std::string NEXT_COMMAND_TYPE("NextCommandIssued");

/// String to identify the Button command argument.
static const std::string PREVIOUS_COMMAND_TYPE("PreviousCommandIssued");

/// String to identify the Previous Button of PlaybackController.
static const std::string PREV_BUTTON_ID("PREVIOUS");

/// String to identify the Next Button of PlaybackController.
static const std::string NEXT_BUTTON_ID("NEXT");

/// String to identify the Play/Pause Button of PlaybackController.
static const std::string PLAYPAUSE_BUTTON_ID("PLAY_PAUSE");

/// String to identify the Shuffle Toggle of PlaybackController.
static const std::string SHUFFLE_TOGGLE_ID("SHUFFLE");

/// String to identify the Loop Toggle of PlaybackController.
static const std::string LOOP_TOGGLE_ID("LOOP");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string REPEAT_TOGGLE_ID("REPEAT");

/// String to identify the Skip Forward Button of PlaybackController.
static const std::string SKIP_FORWARD_BUTTON_ID("SKIPFORWARD");

/// String to identify the Skip Backward Button of PlaybackController.
static const std::string SKIP_BACKWARD_BUTTON_ID("SKIPBACKWARD");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string THUMBSUP_TOGGLE_ID("THUMBSUP");

/// String to identify the Repeat Toggle of PlaybackController.
static const std::string THUMBSDOWN_TOGGLE_ID("THUMBSDOWN");

/// Map to match a toggle command id to the corresponding enum value.
static const std::map<std::string, avsCommon::avs::PlaybackToggle> TOGGLE_COMMAND_ID_TO_TOGGLE = {
    {SHUFFLE_TOGGLE_ID, avsCommon::avs::PlaybackToggle::SHUFFLE},
    {LOOP_TOGGLE_ID, avsCommon::avs::PlaybackToggle::LOOP},
    {REPEAT_TOGGLE_ID, avsCommon::avs::PlaybackToggle::REPEAT},
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
        m_playerActivityState{alexaClientSDK::avsCommon::avs::PlayerActivity::FINISHED} {
    m_guiClient = guiClient;
    m_isMicOn = true;
    m_isHoldOccurring = false;
    m_isTapOccurring = false;
    m_isSpeakingOrListening = false;

#ifdef UWP_BUILD
    m_micWrapper = std::dynamic_pointer_cast<alexaSmartScreenSDK::sssdkCommon::NullMicrophone>(micWrapper);
#else
    m_micWrapper = micWrapper;
#endif

    m_micWrapper->startStreamingMicrophoneData();
}

void GUIManager::renderTemplateCard(
    const std::string& jsonPayload,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::RENDER_TEMPLATE;
    m_guiClient->renderTemplateCard(jsonPayload, focusState);
}

void GUIManager::clearTemplateCard() {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::NONE;
    m_guiClient->clearTemplateCard();
}

void GUIManager::renderPlayerInfoCard(
    const std::string& jsonPayload,
    smartScreenSDKInterfaces::AudioPlayerInfo info,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    m_guiClient->renderPlayerInfoCard(jsonPayload, info, focusState);
}

void GUIManager::clearPlayerInfoCard() {
    m_guiClient->clearPlayerInfoCard();
}

void GUIManager::interruptCommandSequence() {
    m_guiClient->interruptCommandSequence();
}

void GUIManager::renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::ALEXA_PRESENTATION;
    m_guiClient->renderDocument(jsonPayload, token, windowId);
}

void GUIManager::clearDocument() {
    m_activeNonPlayerInfoDisplayType = NonPlayerInfoDisplayType::NONE;
    m_guiClient->clearDocument();
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

void GUIManager::handleHoldToTalk() {
    ACSDK_DEBUG9(LX("handleHoldToTalk"));
    m_executor.submit([this]() {
        if (!m_isMicOn) {
            return;
        }
        if (!m_isHoldOccurring) {
            if (m_ssClient->notifyOfHoldToTalkStart(m_holdToTalkAudioProvider).get()) {
                m_isHoldOccurring = true;
            }
        } else {
            m_isHoldOccurring = false;
            m_ssClient->notifyOfHoldToTalkEnd();
        }
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

void GUIManager::handleUserEvent(std::string userEventPayload) {
    m_executor.submit([this, userEventPayload]() {
        rapidjson::Document document;
        rapidjson::ParseResult result = document.Parse(userEventPayload);

        ACSDK_DEBUG5(LX(__func__).d("payload", userEventPayload));

        if (!result) {
            ACSDK_ERROR(LX(__func__).d("reason", "Payload parse error."));
            return;
        }

        do {
            if (!document.HasMember(ARGUMENTS_TAG)) {
                sendUserEvent(userEventPayload);
                break;
            }

            auto argumentsValue = document.FindMember(ARGUMENTS_TAG);

            if (!argumentsValue->value.IsArray()) {
                sendUserEvent(userEventPayload);
                break;
            }

            auto argumentsArray = argumentsValue->value.GetArray();
            if (argumentsArray.Size() < 2) {
                sendUserEvent(userEventPayload);
                break;
            }

            if (!(argumentsArray[COMMAND_TYPE_INDEX].IsString()) || !(argumentsArray[COMMAND_ID_INDEX].IsString())) {
                sendUserEvent(userEventPayload);
                break;
            }

            std::string commandType = argumentsArray[COMMAND_TYPE_INDEX].GetString();
            std::string commandId = argumentsArray[COMMAND_ID_INDEX].GetString();

            if (commandType == PLAYPAUSE_COMMAND_TYPE && commandId == PLAYPAUSE_BUTTON_ID) {
                playbackPlay();
            } else if (commandType == NEXT_COMMAND_TYPE && commandId == NEXT_BUTTON_ID) {
                playbackNext();
            } else if (commandType == PREVIOUS_COMMAND_TYPE && commandId == PREV_BUTTON_ID) {
                playbackPrevious();
            } else if (commandType == BUTTON_COMMAND_TYPE) {
                if (commandId == SKIP_FORWARD_BUTTON_ID) {
                    playbackSkipForward();
                } else if (commandId == SKIP_BACKWARD_BUTTON_ID) {
                    playbackSkipBackward();
                }
            } else if (commandType == TOGGLE_COMMAND_TYPE) {
                if (argumentsArray.Size() < 3 || !argumentsArray[COMMAND_ACTION_INDEX].IsString()) {
                    ACSDK_INFO(LX(__func__)
                                   .d("Possible error", "Command type is toggle with mismatch arguments")
                                   .d("Payload:", userEventPayload));
                    break;
                }
                std::string actionString = argumentsArray[COMMAND_ACTION_INDEX].GetString();
                bool action = actionString == "SELECT";

                auto it = TOGGLE_COMMAND_ID_TO_TOGGLE.find(commandId);
                if (it == TOGGLE_COMMAND_ID_TO_TOGGLE.end()) {
                    ACSDK_ERROR(LX(__func__).d("Key", commandId));
                    return;
                }
                sendGuiToggleEvent(it->second, action);
            } else {
                sendUserEvent(userEventPayload);
            }
        } while (0);
    });
}

void GUIManager::handleDataSourceFetchRequestEvent(std::string type, std::string payload) {
    m_executor.submit([this, type, payload] { m_ssClient->sendDataSourceFetchRequestEvent(type, payload); });
}

void GUIManager::handleRuntimeErrorEvent(std::string payload) {
    m_executor.submit([this, payload] { m_ssClient->sendRuntimeErrorEvent(payload); });
}

void GUIManager::sendUserEvent(const std::string& payload) {
    m_executor.submit([this, payload]() { m_ssClient->sendUserEvent(payload); });
}

void GUIManager::playbackPlay() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PLAY); });
}

void GUIManager::playbackPause() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PAUSE); });
}

void GUIManager::playbackNext() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::NEXT); });
}

void GUIManager::playbackPrevious() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::PREVIOUS); });
}

void GUIManager::playbackSkipForward() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::SKIP_FORWARD); });
}

void GUIManager::playbackSkipBackward() {
    m_executor.submit(
        [this]() { m_ssClient->getPlaybackRouter()->buttonPressed(avsCommon::avs::PlaybackButton::SKIP_BACKWARD); });
}

void GUIManager::playbackShuffle() {
    sendGuiToggleEvent(avsCommon::avs::PlaybackToggle::SHUFFLE, false);
}

void GUIManager::playbackLoop() {
    sendGuiToggleEvent(avsCommon::avs::PlaybackToggle::LOOP, false);
}

void GUIManager::playbackRepeat() {
    sendGuiToggleEvent(avsCommon::avs::PlaybackToggle::REPEAT, false);
}

void GUIManager::playbackThumbsUp() {
    sendGuiToggleEvent(avsCommon::avs::PlaybackToggle::THUMBS_UP, false);
}

void GUIManager::playbackThumbsDown() {
    sendGuiToggleEvent(avsCommon::avs::PlaybackToggle::THUMBS_DOWN, false);
}

void GUIManager::sendGuiToggleEvent(avsCommon::avs::PlaybackToggle toggleType, const bool action) {
    m_executor.submit(
        [this, toggleType, action]() { m_ssClient->getPlaybackRouter()->togglePressed(toggleType, action); });
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

void GUIManager::handleVisualContext(uint64_t token, std::string payload) {
    m_executor.submit([this, token, payload]() { m_ssClient->handleVisualContext(token, payload); });
}

bool GUIManager::handleFocusAcquireRequest(
    std::string channelName,
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_executor
        .submit([this, channelName, channelObserver]() {
            return m_ssClient->getAudioFocusManager()->acquireChannel(channelName, channelObserver, APL_INTERFACE);
        })
        .get();
}

bool GUIManager::handleFocusReleaseRequest(
    std::string channelName,
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_executor
        .submit([this, channelName, channelObserver]() {
            return m_ssClient->getAudioFocusManager()->releaseChannel(channelName, channelObserver).get();
        })
        .get();
}

void GUIManager::handleRenderDocumentResult(std::string token, bool result, std::string error) {
    m_executor.submit([this, token, result, error]() { m_ssClient->handleRenderDocumentResult(token, result, error); });
}

void GUIManager::handleExecuteCommandsResult(std::string token, bool result, std::string error) {
    m_executor.submit(
        [this, token, result, error]() { m_ssClient->handleExecuteCommandsResult(token, result, error); });
}

void GUIManager::handleActivityEvent(const std::string& source, smartScreenSDKInterfaces::ActivityEvent event) {
    m_executor.submit([this, source, event]() {
        m_ssClient->handleActivityEvent(
            source, event, NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
    });
}

void GUIManager::handleActivityEvent(smartScreenSDKInterfaces::ActivityEvent event) {
    m_executor.submit([this, event]() {
        if (smartScreenSDKInterfaces::ActivityEvent::INTERRUPT == event && m_isSpeakingOrListening) {
            ACSDK_DEBUG3(LX(__func__).d(
                "Interrupted activity while speaking or listening",
                smartScreenSDKInterfaces::activityEventToString(event)));
            executeStopForegroundActivity();
        }
        m_ssClient->handleActivityEvent(
            TAG, event, NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
    });
}

void GUIManager::handleNavigationEvent(smartScreenSDKInterfaces::NavigationEvent event) {
    m_executor.submit([this, event]() {
        /**
         * If we've resumed playing audio and are still presenting GUI over PlayerInfo, only clear the remaining card,
         * Don't Stop ForegroundActivity, as that will kill music
         */
        bool clearCardOnly = alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING == m_playerActivityState &&
                             NonPlayerInfoDisplayType::NONE != m_activeNonPlayerInfoDisplayType;

        if (!clearCardOnly) {
            executeStopForegroundActivity();
        }

        ACSDK_DEBUG3(LX(__func__).d(
            "processNavigationEvent in executor", smartScreenSDKInterfaces::navigationEventToString(event)));
        switch (event) {
            /// Phase 1 - BACK and EXIT will have the same result - EXIT the screen and clear any resources.
            case smartScreenSDKInterfaces::NavigationEvent::BACK:
            case smartScreenSDKInterfaces::NavigationEvent::EXIT:
                if (clearCardOnly) {
                    m_ssClient->clearCard();
                } else {
                    m_ssClient->forceExit();
                }
                break;
            default:
                // Not possible as returned above.
                break;
        }
    });
}

void GUIManager::forceExit() {
    m_executor.submit([this]() { m_ssClient->forceExit(); });
}

void GUIManager::setDocumentIdleTimeout(std::chrono::milliseconds timeout) {
    m_executor.submit([this, timeout]() { m_ssClient->setDocumentIdleTimeout(timeout); });
}

void GUIManager::handleDeviceWindowState(std::string payload) {
    m_executor.submit([this, payload]() { m_ssClient->setDeviceWindowState(payload); });
}

void GUIManager::handleRenderComplete() {
    m_executor.submit([this]() {
        m_ssClient->handleRenderComplete(
            NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
    });
}

void GUIManager::handleDisplayMetrics(uint64_t dropFrameCount) {
    m_executor.submit([this, dropFrameCount]() {
        m_ssClient->handleDropFrameCount(
            dropFrameCount, NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
    });
}

void GUIManager::handleAPLEvent(APLClient::AplRenderingEvent event) {
    m_ssClient->handleAPLEvent(event, NonPlayerInfoDisplayType::ALEXA_PRESENTATION == m_activeNonPlayerInfoDisplayType);
}

void GUIManager::executeStopForegroundActivity() {
    m_ssClient->stopForegroundActivity();
    // TODO: ARC-570 consider cleaning the commands on CommandsSequencer
    m_ssClient->clearAllExecuteCommands();
}

void GUIManager::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
}

void GUIManager::onCapabilitiesStateChange(
    CapabilitiesObserverInterface::State newState,
    CapabilitiesObserverInterface::Error newError,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& addedOrUpdatedEndpoints,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& deletedEndpoints) {
}

void GUIManager::provideState(const unsigned int stateRequestToken) {
    m_executor.submit([this, stateRequestToken]() { m_guiClient->provideState(stateRequestToken); });
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

void GUIManager::onPlayerActivityChanged(avsCommon::avs::PlayerActivity state, const Context& context) {
    m_executor.submit([this, state]() { m_playerActivityState = state; });
}

void GUIManager::setClient(std::shared_ptr<smartScreenClient::SmartScreenClient> client) {
    m_executor.submit([this, client]() {
        if (!client) {
            ACSDK_CRITICAL(LX(__func__).d("reason", "null client"));
        }
        m_ssClient = client;
    });
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
