/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <memory>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>

#include "LiveViewControllerCapabilityAgent/LiveViewControllerCapabilityAgent.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace liveViewController {

using namespace alexaClientSDK;
using namespace alexaSmartScreenSDK;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;
using namespace avsCommon::utils::configuration;
using namespace avsCommon::utils::json;

/// LiveViewControllerCapabilityAgent name.
static const std::string LIVEVIEWCONTROLLER_CAPABILITY_AGENT = "LiveViewControllerCapabilityAgent";

/// String to identify log entries originating from this file.
static const std::string TAG("LiveViewControllerCapabilityAgent");

/// LiveViewControllerCapabilityAgent interface type
static const std::string LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";

/// LiveViewControllerCapabilityAgent interface name
static const std::string LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME = "Alexa.Camera.LiveViewController";

/// LiveViewControllerCapabilityAgent interface version for Alexa.Camera.LiveViewController
static const std::string LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_VERSION = "1.7";

/// The @c StartLiveView directive signature.
static const NamespaceAndName START_LIVE_VIEW{LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME, "StartLiveView"};

/// The @c StopLiveView directive signature.
static const NamespaceAndName STOP_LIVE_VIEW{LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME, "StopLiveView"};

/// Identifier for the SessionId sent in a StartLiveView directive
static const std::string SESSION_ID_FIELD = "sessionId";

/// Identifier for the Target sent in a StartLiveView directive
static const std::string TARGET_FIELD = "target";

/// Identifier for the Role sent in a StartLiveView directive
static const std::string ROLE_FIELD = "role";

/// Identifier for the participants sent in a StartLiveView directive
static const std::string PARTICIPANTS_FIELD = "participants";

/// Identifier for the viewerExperience sent in a StartLiveView directive
static const std::string VIEWER_EXPERIENCE_FIELD = "viewerExperience";

/// Identifier for a viewerExperience's audioProperties sent in a StartLiveView directive
static const std::string AUDIO_PROPERTIES_FIELD = "audioProperties";

/// Identifier for an audioProperties's concurrentTwoWayTalk sent in a StartLiveView directive
static const std::string CONCURRENT_TWO_WAY_TALK_FIELD = "concurrentTwoWayTalk";

/// Identifier for an audioProperties's microphoneState sent in a StartLiveView directive
static const std::string MICROPHONE_STATE_FIELD = "microphoneState";

/// Identifier for a target's type sent in a StartLiveView directive
static const std::string TYPE_FIELD = "type";

/// Identifier for a target's endpointId sent in a StartLiveView directive
static const std::string ENDPOINT_ID_FIELD = "endpointId";

/// App identifier sent to RTCSessionController App Client.
static const std::string APP_IDENTIFIER = "SmartHome-LiveView";

/// LiveViewStarted event name.
static const std::string LIVE_VIEW_STARTED_EVENT_NAME = "LiveViewStarted";

/// LiveViewStopped event name.
static const std::string LIVE_VIEW_STOPPED_EVENT_NAME = "LiveViewStopped";

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The name of the @c FocusManager channel used by @c LiveViewController CA.
static const std::string CHANNEL_NAME = avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_NAME;

std::shared_ptr<LiveViewControllerCapabilityAgent> LiveViewControllerCapabilityAgent::create(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) {
    if (!focusManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
        return nullptr;
    }

    if (!messageSender) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
        return nullptr;
    }

    if (!contextManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManager"));
        return nullptr;
    }

    if (!exceptionSender) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
        return nullptr;
    }

    std::shared_ptr<LiveViewControllerCapabilityAgent> liveViewControllerCapabilityAgent(
        new LiveViewControllerCapabilityAgent(focusManager, messageSender, contextManager, exceptionSender));

    return liveViewControllerCapabilityAgent;
}

LiveViewControllerCapabilityAgent::LiveViewControllerCapabilityAgent(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) :
        CapabilityAgent{LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME, exceptionSender},
        RequiresShutdown{LIVEVIEWCONTROLLER_CAPABILITY_AGENT},
        m_messageSender{messageSender},
        m_contextManager{contextManager},
        m_focusManager{focusManager},
        m_focus{FocusState::NONE},
        m_state{smartScreenSDKInterfaces::State::IDLE},
        m_concurrentTwoWayTalk{smartScreenSDKInterfaces::ConcurrentTwoWayTalk::DISABLED},
        m_microphoneState{smartScreenSDKInterfaces::AudioState::MUTED} {
    m_executor = std::make_shared<alexaClientSDK::avsCommon::utils::threading::Executor>();
    m_appInfo = rtc::native::AppInfo{APP_IDENTIFIER};
    m_capabilityConfigurations.insert(getLiveViewControllerCapabilityConfiguration());
}

void LiveViewControllerCapabilityAgent::handleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
    ACSDK_DEBUG5(LX(__func__));
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void LiveViewControllerCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    // no-op
}

void LiveViewControllerCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    if (!info || !info->directive) {
        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
        return;
    }

    ACSDK_DEBUG5(LX("handleDirective")
                     .d("name", info->directive->getName())
                     .d("messageId", info->directive->getMessageId())
                     .d("correlationToken", info->directive->getCorrelationToken()));

    if (info->directive->getNamespace() == START_LIVE_VIEW.nameSpace &&
        info->directive->getName() == START_LIVE_VIEW.name) {
        handleStartLiveView(info);
    } else if (
        info->directive->getNamespace() == STOP_LIVE_VIEW.nameSpace &&
        info->directive->getName() == STOP_LIVE_VIEW.name) {
        handleStopLiveView(info);
    } else {
        handleUnknownDirective(info);
    }
}

void LiveViewControllerCapabilityAgent::handleStartLiveView(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));

    m_executor->submit([this, info]() {
        ACSDK_DEBUG9(LX("handleStartLiveViewInExecutor").sensitive("payload", info->directive->getPayload()));
        rapidjson::Document payload;
        if (!parseDirectivePayload(info, &payload)) {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "NoPayload"));
            sendExceptionEncounteredAndReportFailed(info, "NoPayload");
            return;
        }

        std::string sessionId;
        if (!jsonUtils::retrieveValue(payload, SESSION_ID_FIELD, &sessionId)) {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "NoSessionId"));
            sendExceptionEncounteredAndReportFailed(info, "missing sessionId");
            return;
        }

        rapidjson::Value::ConstMemberIterator target;
        std::string targetParsedPayload;
        if (jsonUtils::findNode(payload, TARGET_FIELD, &target) &&
            jsonUtils::convertToValue(target->value, &targetParsedPayload)) {
            rapidjson::Document doc;
            doc.Parse(targetParsedPayload);

            if (doc.HasMember(ENDPOINT_ID_FIELD) && doc[ENDPOINT_ID_FIELD].IsString()) {
                m_targetEndpointId = doc[ENDPOINT_ID_FIELD].GetString();
            } else {
                ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "NoEndpointId"));
                sendExceptionEncounteredAndReportFailed(info, "missing EndpointId");
                return;
            }

            if (doc.HasMember(TYPE_FIELD) && doc[TYPE_FIELD].IsString()) {
                m_targetType = doc[TYPE_FIELD].GetString();
            } else {
                ACSDK_WARN(LX("handleStartLiveViewInExecutor").m("Cannot find valid type"));
            }
        } else {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "NoTarget"));
            sendExceptionEncounteredAndReportFailed(info, "missing Target");
        }

        std::string role;
        if (!jsonUtils::retrieveValue(payload, ROLE_FIELD, &role)) {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "NoRole"));
            sendExceptionEncounteredAndReportFailed(info, "missing role");
            return;
        } else if (smartScreenSDKInterfaces::roleFromString(role) != smartScreenSDKInterfaces::Role::VIEWER) {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "only supporting viewer role"));
            sendExceptionEncounteredAndReportFailed(info, "only supporting viewer role");
            return;
        }

        rapidjson::Value::ConstMemberIterator participantsIterator;
        if (!jsonUtils::findNode(payload, PARTICIPANTS_FIELD, &participantsIterator)) {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "MissingParticipants"));
            sendExceptionEncounteredAndReportFailed(info, "missing Participants");
            return;
        }

        rapidjson::Value::ConstMemberIterator viewerExperienceIterator;
        std::string viewerExperiencePayload;
        m_concurrentTwoWayTalk = smartScreenSDKInterfaces::ConcurrentTwoWayTalk::DISABLED;
        m_microphoneState = smartScreenSDKInterfaces::AudioState::MUTED;
        if (jsonUtils::findNode(payload, VIEWER_EXPERIENCE_FIELD, &viewerExperienceIterator) &&
            jsonUtils::convertToValue(viewerExperienceIterator->value, &viewerExperiencePayload)) {
            rapidjson::Document doc;
            doc.Parse(viewerExperiencePayload);

            if (doc.HasMember(AUDIO_PROPERTIES_FIELD) && doc[AUDIO_PROPERTIES_FIELD].IsObject()) {
                auto audioPropertiesObject = doc[AUDIO_PROPERTIES_FIELD].GetObject();

                if (audioPropertiesObject.HasMember(CONCURRENT_TWO_WAY_TALK_FIELD) &&
                    audioPropertiesObject[CONCURRENT_TWO_WAY_TALK_FIELD].IsString()) {
                    m_concurrentTwoWayTalk = smartScreenSDKInterfaces::concurrentTwoWayTalkFromString(
                        audioPropertiesObject[CONCURRENT_TWO_WAY_TALK_FIELD].GetString());
                } else {
                    ACSDK_WARN(LX("handleStartLiveViewInExecutor").m("Cannot find valid concurrentTwoWayTalk"));
                }

                if (audioPropertiesObject.HasMember(MICROPHONE_STATE_FIELD) &&
                    audioPropertiesObject[MICROPHONE_STATE_FIELD].IsString()) {
                    m_microphoneState = smartScreenSDKInterfaces::audioStateFromString(
                        audioPropertiesObject[MICROPHONE_STATE_FIELD].GetString());
                } else {
                    ACSDK_WARN(LX("handleStartLiveViewInExecutor").m("Cannot find valid microphoneState"));
                }
            } else {
                ACSDK_WARN(LX("handleStartLiveViewInExecutor").m("Cannot find valid audioProperties"));
            }
        } else {
            ACSDK_WARN(LX("handleStartLiveViewInExecutor").m("Cannot find valid viewerExperience"));
        }

        rapidjson::Document liveViewStartedPayload(rapidjson::kObjectType);
        auto& alloc = liveViewStartedPayload.GetAllocator();

        rapidjson::Document targetPayload(rapidjson::kObjectType, &alloc);
        targetPayload.AddMember(rapidjson::StringRef(ENDPOINT_ID_FIELD), m_targetEndpointId, alloc);
        targetPayload.AddMember(rapidjson::StringRef(TYPE_FIELD), m_targetType, alloc);

        liveViewStartedPayload.AddMember(rapidjson::StringRef(SESSION_ID_FIELD), sessionId, alloc);
        liveViewStartedPayload.AddMember(rapidjson::StringRef(TARGET_FIELD), targetPayload, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        if (liveViewStartedPayload.Accept(writer)) {
            executeSendLiveViewEvent(LIVE_VIEW_STARTED_EVENT_NAME, buffer.GetString());
        } else {
            ACSDK_ERROR(LX("handleStartLiveViewInExecutor").d("reason", "writerRefusedJsonObject"));
        }

        executeInstantiateRtcscAppClient();

        // We only allow one active session at time with the Rtcsc Client, so disconnect the current session if active.
        if (!m_lastSessionId.empty() && m_lastSessionId != sessionId) {
            ACSDK_DEBUG5(LX("handleStartLiveViewInExecutor").d("interrupt session", "session id changed"));
            executeDisconnectRtcscSession(
                m_lastSessionId, rtc::native::RtcscAppDisconnectCode::HIGHER_PRIORITY_SESSION_INTERRUPTED);
        }
        m_lastSessionId = sessionId;

        executeStartLiveViewDirective(info);
        setHandlingCompleted(info);
    });
}

void LiveViewControllerCapabilityAgent::executeStartLiveViewDirective(
    const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    smartScreenSDKInterfaces::State nextState = m_state;
    m_lastDisplayedDirective = info;

    if (!m_rtcscAppClient) {
        ACSDK_ERROR(LX("executeStartLiveViewDirective").d("reason", "Null rtcscAppClient"));
        sendExceptionEncounteredAndReportFailed(info, "Null rtcscAppClient", ExceptionErrorType::INTERNAL_ERROR);
        m_lastDisplayedDirective.reset();
        return;
    }

    auto errorCode = m_rtcscAppClient->registerAppClientListener(m_appInfo, this);

    if (errorCode != rtc::native::RtcscErrorCode::SUCCESS) {
        ACSDK_ERROR(LX(__func__).d("registerAppClientListener RtcscErrorCode", rtc::native::toString(errorCode)));
        const std::string exceptionMessage = "registerAppClientListener results in " + rtc::native::toString(errorCode);
        sendExceptionEncounteredAndReportFailed(info, exceptionMessage, ExceptionErrorType::INTERNAL_ERROR);
        m_lastDisplayedDirective.reset();
        return;
    }

    switch (m_state) {
        case smartScreenSDKInterfaces::State::IDLE:
            m_focusHoldingInterface = m_lastDisplayedDirective->directive->getNamespace();
            m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), m_focusHoldingInterface);
            nextState = smartScreenSDKInterfaces::State::ACQUIRING;
            break;
        case smartScreenSDKInterfaces::State::ACQUIRING:
            // Do Nothing.
            break;
        case smartScreenSDKInterfaces::State::DISPLAYING:
            if (m_focusHoldingInterface == m_lastDisplayedDirective->directive->getNamespace()) {
                executeRenderLiveView();
                nextState = smartScreenSDKInterfaces::State::DISPLAYING;
            } else {
                nextState = smartScreenSDKInterfaces::State::REACQUIRING;
                m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            }
            break;
        case smartScreenSDKInterfaces::State::RELEASING:
            nextState = smartScreenSDKInterfaces::State::REACQUIRING;
            break;
        case smartScreenSDKInterfaces::State::REACQUIRING:
            // Do Nothing.
            break;
    }
    ACSDK_DEBUG5(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void LiveViewControllerCapabilityAgent::executeRenderLiveView() {
    ACSDK_DEBUG5(LX(__func__));

    if (hasActiveLiveView()) {
        executeRenderLiveViewCallbacks(false);
    }
}

void LiveViewControllerCapabilityAgent::executeRenderLiveViewCallbacks(bool isClearLiveView) {
    ACSDK_DEBUG5(LX(__func__));
    if (!isClearLiveView) {
        executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::CONNECTING);
        for (auto& observer : m_observers) {
            observer->renderCamera(
                m_lastDisplayedDirective->directive->getPayload(), m_microphoneState, m_concurrentTwoWayTalk);
        }
    } else {
        executeDisconnectRtcscSession(m_lastSessionId, rtc::native::RtcscAppDisconnectCode::USER_TERMINATED_SESSION);
        m_lastDisplayedDirective = nullptr;
        for (auto& observer : m_observers) {
            observer->clearCamera();
        }

        rapidjson::Document targetPayload(rapidjson::kObjectType);
        targetPayload.AddMember(
            rapidjson::StringRef(ENDPOINT_ID_FIELD), m_targetEndpointId, targetPayload.GetAllocator());
        targetPayload.AddMember(rapidjson::StringRef(TYPE_FIELD), m_targetType, targetPayload.GetAllocator());

        rapidjson::Document liveViewStoppedPayload(rapidjson::kObjectType);
        liveViewStoppedPayload.AddMember(
            rapidjson::StringRef(SESSION_ID_FIELD), m_lastSessionId, liveViewStoppedPayload.GetAllocator());
        liveViewStoppedPayload.AddMember(
            rapidjson::StringRef(TARGET_FIELD), targetPayload, liveViewStoppedPayload.GetAllocator());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        if (liveViewStoppedPayload.Accept(writer)) {
            executeSendLiveViewEvent(LIVE_VIEW_STOPPED_EVENT_NAME, buffer.GetString());
        } else {
            ACSDK_ERROR(LX(__func__).d("reason", "writerRefusedJsonObject"));
        }
        m_targetType = {};
        m_targetEndpointId = {};
    }
}

void LiveViewControllerCapabilityAgent::handleStopLiveView(const std::shared_ptr<DirectiveInfo> info) {
    // TODO we clear the current live view regardless of StopLiveView directive content due to an issue from SH cloud
    ACSDK_DEBUG5(LX(__func__));
    m_executor->submit([this, info]() {
        ACSDK_DEBUG9(LX("handleStopLiveViewInExecutor").sensitive("payload", info->directive->getPayload()));
        rapidjson::Document payload;
        if (!parseDirectivePayload(info, &payload)) {
            ACSDK_ERROR(LX("handleStopLiveViewInExecutor").d("reason", "NoPayload"));
        }

        rapidjson::Value::ConstMemberIterator target;
        std::string targetParsedPayload;
        std::string targetEndpointId;
        std::string targetType;
        if (jsonUtils::findNode(payload, TARGET_FIELD, &target) &&
            jsonUtils::convertToValue(target->value, &targetParsedPayload)) {
            rapidjson::Document doc;
            doc.Parse(targetParsedPayload);

            if (doc.HasMember(ENDPOINT_ID_FIELD) && doc[ENDPOINT_ID_FIELD].IsString()) {
                targetEndpointId = doc[ENDPOINT_ID_FIELD].GetString();
            } else {
                ACSDK_ERROR(LX("handleStopLiveViewInExecutor").d("reason", "NoEndpointId"));
            }

            if (targetEndpointId != m_targetEndpointId) {
                ACSDK_ERROR(LX("handleStopLiveViewInExecutor")
                                .d("reason", "mismatchedEndpointId")
                                .d("expectedEndpointId", m_targetEndpointId)
                                .d("receivedEndpointId", targetEndpointId));
            }

            if (doc.HasMember(TYPE_FIELD) && doc[TYPE_FIELD].IsString()) {
                targetType = doc[TYPE_FIELD].GetString();
            } else {
                ACSDK_WARN(LX("handleStopLiveViewInExecutor").m("Cannot find valid type"));
            }

            if (!targetType.empty() && targetType != m_targetType) {
                ACSDK_ERROR(LX("handleStopLiveViewInExecutor")
                                .d("reason", "mismatchedType")
                                .d("expectedType", m_targetType)
                                .d("receivedType", targetType));
            }
        } else {
            ACSDK_ERROR(LX("handleStopLiveViewInExecutor").d("reason", "NoTarget"));
        }

        executeStopLiveViewDirective(info);
        setHandlingCompleted(info);
    });
}

void LiveViewControllerCapabilityAgent::executeStopLiveViewDirective(
    const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info) {
    executeClearLiveViewEvent();
}

void LiveViewControllerCapabilityAgent::executeClearLiveView() {
    ACSDK_DEBUG5(LX(__func__));
    if (hasActiveLiveView()) {
        executeRenderLiveViewCallbacks(true);
    }
}

void LiveViewControllerCapabilityAgent::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_ERROR(LX("requestedToHandleUnknownDirective")
                    .d("reason", "unknownDirective")
                    .d("namespace", info->directive->getNamespace())
                    .d("name", info->directive->getName()));

    m_executor->submit([this, info] {
        const std::string exceptionMessage =
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();

        sendExceptionEncounteredAndReportFailed(
            info, exceptionMessage, ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    });
}

void LiveViewControllerCapabilityAgent::executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState cameraState) {
    ACSDK_DEBUG5(LX(__func__));
    for (auto& observer : m_observers) {
        observer->onCameraStateChanged(cameraState);
    }
}

bool LiveViewControllerCapabilityAgent::parseDirectivePayload(
    std::shared_ptr<DirectiveInfo> info,
    rapidjson::Document* document) {
    rapidjson::ParseResult result = document->Parse(info->directive->getPayload());
    if (result) {
        return true;
    } else {
        ACSDK_ERROR(LX("parseDirectivePayloadFailed")
                        .d("reason", rapidjson::GetParseError_En(result.Code()))
                        .d("offset", result.Offset())
                        .d("messageId", info->directive->getMessageId()));
        sendExceptionEncounteredAndReportFailed(
            info, "Unable to parse payload", ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return false;
    }
}

void LiveViewControllerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    removeDirective(info);
}

DirectiveHandlerConfiguration LiveViewControllerCapabilityAgent::getConfiguration() const {
    ACSDK_DEBUG5(LX(__func__));
    DirectiveHandlerConfiguration configuration;

    configuration[START_LIVE_VIEW] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
    configuration[STOP_LIVE_VIEW] = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);

    return configuration;
}

void LiveViewControllerCapabilityAgent::onFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newFocus,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    ACSDK_DEBUG5(LX(__func__));
    m_executor->submit([this, newFocus]() { executeOnFocusChangedEvent(newFocus); });
}

void LiveViewControllerCapabilityAgent::executeClearLiveViewEvent() {
    smartScreenSDKInterfaces::State nextState = m_state;

    switch (m_state) {
        case smartScreenSDKInterfaces::State::DISPLAYING:
            executeClearLiveView();
            m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
            nextState = smartScreenSDKInterfaces::State::RELEASING;
            break;

        case smartScreenSDKInterfaces::State::IDLE:
        case smartScreenSDKInterfaces::State::ACQUIRING:
        case smartScreenSDKInterfaces::State::RELEASING:
        case smartScreenSDKInterfaces::State::REACQUIRING:
            // Do Nothing.
            break;
    }
    ACSDK_DEBUG5(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void LiveViewControllerCapabilityAgent::executeOnFocusChangedEvent(
    alexaClientSDK::avsCommon::avs::FocusState newFocus) {
    ACSDK_DEBUG5(LX(__func__).d("prevFocus", m_focus).d("newFocus", newFocus));

    bool weirdFocusState = false;
    smartScreenSDKInterfaces::State nextState = m_state;
    m_focus = newFocus;

    switch (m_state) {
        case smartScreenSDKInterfaces::State::IDLE:
            // This is weird.  We shouldn't be getting any focus updates in Idle.
            switch (newFocus) {
                case FocusState::FOREGROUND:
                case FocusState::BACKGROUND:
                    weirdFocusState = true;
                    break;
                case FocusState::NONE:
                    // Do nothing.
                    break;
            }
            break;
        case smartScreenSDKInterfaces::State::ACQUIRING:
            switch (newFocus) {
                case FocusState::FOREGROUND:
                case FocusState::BACKGROUND:
                    executeRenderLiveView();
                    nextState = smartScreenSDKInterfaces::State::DISPLAYING;
                    break;
                case FocusState::NONE:
                    ACSDK_ERROR(LX("executeOnFocusChangedEventFailed")
                                    .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                                    .d("nextFocus", newFocus)
                                    .d("reason", "Unexpected focus state event."));
                    nextState = smartScreenSDKInterfaces::State::IDLE;
                    break;
            }
            break;
        case smartScreenSDKInterfaces::State::DISPLAYING:
            switch (newFocus) {
                case FocusState::FOREGROUND:
                case FocusState::BACKGROUND:
                    executeRenderLiveView();
                    break;
                case FocusState::NONE:
                    executeClearLiveView();
                    nextState = smartScreenSDKInterfaces::State::IDLE;
                    break;
            }
            break;
        case smartScreenSDKInterfaces::State::RELEASING:
            switch (newFocus) {
                case FocusState::FOREGROUND:
                case FocusState::BACKGROUND:
                    weirdFocusState = true;
                    break;
                case FocusState::NONE:
                    nextState = smartScreenSDKInterfaces::State::IDLE;
                    break;
            }
            break;
        case smartScreenSDKInterfaces::State::REACQUIRING:
            switch (newFocus) {
                case FocusState::FOREGROUND:
                case FocusState::BACKGROUND:
                    weirdFocusState = true;
                    break;
                case FocusState::NONE:
                    m_focusManager->acquireChannel(CHANNEL_NAME, shared_from_this(), m_focusHoldingInterface);
                    nextState = smartScreenSDKInterfaces::State::ACQUIRING;
                    break;
            }
            break;
    }
    if (weirdFocusState) {
        ACSDK_ERROR(LX("executeOnFocusChangedEventFailed")
                        .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                        .d("nextFocus", newFocus)
                        .d("reason", "Unexpected focus state event."));
        m_focusManager->releaseChannel(CHANNEL_NAME, shared_from_this());
        nextState = smartScreenSDKInterfaces::State::RELEASING;
    }
    ACSDK_DEBUG5(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

std::unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> LiveViewControllerCapabilityAgent::
    getCapabilityConfigurations() {
    ACSDK_DEBUG5(LX(__func__));
    return m_capabilityConfigurations;
}

std::shared_ptr<CapabilityConfiguration> LiveViewControllerCapabilityAgent::
    getLiveViewControllerCapabilityConfiguration() {
    ACSDK_DEBUG5(LX(__func__));
    std::unordered_map<std::string, std::string> configMap;

    configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME});
    configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<CapabilityConfiguration>(configMap);
}

void LiveViewControllerCapabilityAgent::doShutdown() {
    ACSDK_DEBUG5(LX(__func__));
    m_executor->shutdown();
    m_messageSender.reset();
    m_contextManager.reset();
    m_focusManager.reset();
    m_observers.clear();
}

void LiveViewControllerCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    if (info) {
        if (info->result) {
            info->result->setCompleted();
        }
        removeDirective(info);
    }
}

void LiveViewControllerCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if (info->directive && info->result) {
        CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void LiveViewControllerCapabilityAgent::setExecutor(
    const std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor>& executor) {
    ACSDK_WARN(LX(__func__).d("reason", "should be called in test only"));
    m_executor = executor;
}

void LiveViewControllerCapabilityAgent::setRtcscAppClient(
    const std::shared_ptr<rtc::native::RtcscAppClientInterface>& rtcscAppClient) {
    ACSDK_WARN(LX(__func__).d("reason", "should be called in test only"));
    m_rtcscAppClient = rtcscAppClient;
}

void LiveViewControllerCapabilityAgent::addObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface>
        observer) {
    ACSDK_DEBUG5(LX(__func__));
    if (!observer) {
        ACSDK_ERROR(LX("addObserverFailed").d("reason", "Observer is null."));
        return;
    }
    m_executor->submit([this, observer]() {
        ACSDK_DEBUG5(LX("addObserverInExecutor"));
        if (!m_observers.insert(observer).second) {
            ACSDK_ERROR(LX("addObserverFailedInExecutor").d("reason", "Duplicate observer."));
        }
    });
}

void LiveViewControllerCapabilityAgent::removeObserver(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface>
        observer) {
    ACSDK_DEBUG5(LX(__func__));
    if (!observer) {
        ACSDK_ERROR(LX("removeObserverFailed").d("reason", "Observer is null."));
        return;
    }
    m_executor->submit([this, observer]() {
        ACSDK_DEBUG5(LX("removeObserverInExecutor"));
        if (m_observers.erase(observer) == 0) {
            ACSDK_WARN(LX("removeObserverInExecutor").d("reason", "Nonexistent observer."));
        }
    });
}

void LiveViewControllerCapabilityAgent::setMicrophoneState(bool enabled) {
    ACSDK_DEBUG5(LX(__func__).d("micEnabled", enabled));

    m_executor->submit([this, enabled]() {
        if (!m_rtcscAppClient) {
            ACSDK_ERROR(LX("setMicrophoneStateFailed").d("reason", "Null rtcscAppClient"));
            return;
        }

        auto result = m_rtcscAppClient->setLocalAudioState(m_lastSessionId, enabled);
        if (result != rtc::native::RtcscErrorCode::SUCCESS) {
            ACSDK_ERROR(LX("setMicrophoneStateInExecutor")
                            .d("reason", "setLocalAudioStateFailed")
                            .d("result", toString(result)));
        }
        if (m_concurrentTwoWayTalk != smartScreenSDKInterfaces::ConcurrentTwoWayTalk::ENABLED) {
            result = m_rtcscAppClient->setRemoteAudioState(m_lastSessionId, !enabled);
            if (result != rtc::native::RtcscErrorCode::SUCCESS) {
                ACSDK_ERROR(LX("setMicrophoneStateInExecutor")
                                .d("reason", "setRemoteAudioState")
                                .d("result", toString(result)));
            }
        }
    });
}

void LiveViewControllerCapabilityAgent::onSessionAvailable(const std::string& sessionId) {
    ACSDK_DEBUG5(LX(__func__).d("sessionId", sessionId));

    m_executor->submit([this, sessionId]() {
        if (!hasActiveLiveView()) {
            ACSDK_WARN(LX(__func__).d("onSessionAvailableFailedInExecutor", "No active live view directive"));
            return;
        }
        if (sessionId != m_lastSessionId) {
            ACSDK_WARN(LX("onSessionAvailableFailedInExecutor")
                           .d("reason", "Mismatched sessionIds from LiveViewController and RTCSessionController")
                           .d("current SessionId", m_lastSessionId)
                           .d("received SessionId", sessionId));
            return;
        }

        if (!m_rtcscAppClient) {
            ACSDK_ERROR(LX("onSessionAvailableFailedInExecutor").d("reason", "Null rtcscAppClient"));
            return;
        }

        auto result = m_rtcscAppClient->signalReadyForSession(sessionId);

        if (result != rtc::native::RtcscErrorCode::SUCCESS) {
            ACSDK_WARN(LX("onSessionAvailableFailedInExecutor").d("reason", toString(result)));
        }
    });
}

void LiveViewControllerCapabilityAgent::onSessionRemoved(const std::string& sessionId) {
    ACSDK_DEBUG5(LX(__func__).d("sessionId", sessionId));

    m_executor->submit([this, sessionId]() {
        if (!hasActiveLiveView()) {
            // Unregister as an RTC client listener when a session has been removed, and we have no active live view
            // directive.
            ACSDK_DEBUG5(LX("onSessionRemovedInExecutor").d("unregistering app listener", sessionId));
            if (m_rtcscAppClient) {
                auto result = m_rtcscAppClient->unregisterAppClientListener(m_appInfo);
                ACSDK_DEBUG5(LX("onSessionRemovedInExecutor").d("rtcscUnregisterCode", rtc::native::toString(result)));
                if (result != rtc::native::RtcscErrorCode::SUCCESS) {
                    ACSDK_WARN(LX("onSessionRemovedInExecutor").d("reason", toString(result)));
                }
                m_rtcscAppClient.reset();
                rtc::native::RtcscAppClientInterface::releaseInstance();
            }
        }
        if (m_lastSessionId == sessionId) {
            ACSDK_DEBUG5(LX("onSessionRemovedInExecutor").d("reset lastSessionId", sessionId));
            m_lastSessionId = {};
        }
    });
}

void LiveViewControllerCapabilityAgent::onError(
    rtc::native::RtcscErrorCode errorCode,
    const std::string& errorMessage,
    const rtc::native::Optional<std::string>& sessionId) {
    m_executor->submit([this, errorCode, errorMessage, sessionId] {
        ACSDK_DEBUG5(LX(__func__).d("errorCode", rtc::native::toString(errorCode)).d("errorMessage", errorMessage));
        if (!hasActiveLiveView()) {
            ACSDK_WARN(LX(__func__).d("reason", "No active live view directive"));
            return;
        }
        if (sessionId.hasValue() && sessionId.value() != m_lastSessionId) {
            ACSDK_WARN(LX(__func__)
                           .d("reason", "Mismatched sessionIds from LiveViewController and RTCSessionController")
                           .d("current SessionId", m_lastSessionId)
                           .d("received SessionId", sessionId.value()));
            return;
        }
        executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::ERROR);
    });
}

void LiveViewControllerCapabilityAgent::onSessionStateChanged(
    const std::string& sessionId,
    rtc::native::SessionState sessionState) {
    ACSDK_DEBUG5(LX(__func__).d("sessionState", rtc::native::toString(sessionState)).d("sessionId", sessionId));
    // No-op
}

void LiveViewControllerCapabilityAgent::onMediaStatusChanged(
    const std::string& sessionId,
    rtc::native::MediaSide mediaSide,
    rtc::native::MediaType mediaType,
    bool enabled) {
    ACSDK_DEBUG5(LX(__func__)
                     .d("mediaSide", rtc::native::toString(mediaSide))
                     .d("mediaType", rtc::native::toString(mediaType))
                     .d("sessionId", sessionId));
    // No-op
}

void LiveViewControllerCapabilityAgent::onVideoEffectChanged(
    const std::string& sessionId,
    rtc::native::VideoEffect currentVideoEffect,
    int videoEffectDurationMs) {
    ACSDK_DEBUG5(LX(__func__)
                     .d("sessionId", sessionId)
                     .d("currentVideoEffect", rtc::native::toString(currentVideoEffect))
                     .d("videoEffectDurationMs", videoEffectDurationMs));
    // No-op
}

void LiveViewControllerCapabilityAgent::onMediaConnectionStateChanged(
    const std::string& sessionId,
    rtc::native::MediaConnectionState state) {
    m_executor->submit([this, sessionId, state] {
        ACSDK_DEBUG5(LX("onMediaConnectionStateChangedInExecutor").d("state", rtc::native::toString(state)));
        if (!hasActiveLiveView()) {
            ACSDK_WARN(LX("onMediaConnectionStateChangedInExecutor").d("reason", "No active live view directive"));
            return;
        }
        if (sessionId != m_lastSessionId) {
            ACSDK_WARN(LX("onMediaConnectionStateChangedInExecutor")
                           .d("reason", "Mismatched sessionIds from LiveViewController and RTCSessionController")
                           .d("current SessionId", m_lastSessionId)
                           .d("received SessionId", sessionId));
            return;
        }
        switch (state) {
            case rtc::native::MediaConnectionState::CONNECTING:
                executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::CONNECTING);
                break;
            case rtc::native::MediaConnectionState::CONNECTED:
                executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::CONNECTED);
                // TODO: RTCSC Client to provide API for setting mic state on init.
                // For now, always init with mic MUTED, and wait for call from LVC UI to unmute
                setMicrophoneState(false);
                break;
            case rtc::native::MediaConnectionState::DISCONNECTED:
                executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::DISCONNECTED);
                executeClearLiveViewEvent();
                break;
            case rtc::native::MediaConnectionState::UNKNOWN:
                executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState::UNKNOWN);
                break;
        }
    });
}

void LiveViewControllerCapabilityAgent::onFirstFrameReceived(
    const std::string& sessionId,
    rtc::native::MediaType mediaType) {
    ACSDK_DEBUG5(LX(__func__).d("sessionId", sessionId).d("mediaType", rtc::native::toString(mediaType)));
    // No-op
}

void LiveViewControllerCapabilityAgent::onFirstFrameRendered(
    const std::string& sessionId,
    rtc::native::MediaSide mediaSide) {
    ACSDK_DEBUG5(LX(__func__).d("sessionId", sessionId).d("mediaSide", rtc::native::toString(mediaSide)));
    m_executor->submit([this, sessionId] {
        if (!hasActiveLiveView()) {
            ACSDK_WARN(LX("onFirstFrameRenderedInExecutor").d("reason", "No active live view directive"));
            return;
        }
        if (sessionId != m_lastSessionId) {
            ACSDK_WARN(LX("onFirstFrameRenderedInExecutor")
                           .d("reason", "Mismatched sessionIds from LiveViewController and RTCSessionController")
                           .d("current SessionId", m_lastSessionId)
                           .d("received SessionId", sessionId));
            return;
        }
        for (auto& observer : m_observers) {
            observer->onFirstFrameRendered();
        }
    });
}

void LiveViewControllerCapabilityAgent::clearLiveView() {
    ACSDK_DEBUG5(LX(__func__));
    m_executor->submit([this]() { executeClearLiveViewEvent(); });
}

void LiveViewControllerCapabilityAgent::executeSendLiveViewEvent(
    const std::string& eventName,
    const std::string& payload) {
    m_executor->submit([this, eventName, payload]() {
        auto msgIdAndJsonEvent =
            avsCommon::avs::buildJsonEventString(LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME, eventName, "", payload);
        auto userEventMessage = std::make_shared<avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);
        ACSDK_DEBUG9(LX("Sending event to AVS")
                         .d("namespace", LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME)
                         .d("name", eventName));
        m_messageSender->sendMessage(userEventMessage);
    });
}

void LiveViewControllerCapabilityAgent::executeInstantiateRtcscAppClient() {
    if (!m_rtcscAppClient) {
        m_rtcscAppClient = rtc::native::RtcscAppClientInterface::getInstance();
    }
}

void LiveViewControllerCapabilityAgent::executeDisconnectRtcscSession(
    const std::string& sessionId,
    rtc::native::RtcscAppDisconnectCode disconnectCode) {
    ACSDK_DEBUG5(LX(__func__).d("sessionId", sessionId).d("disconnectCode", rtc::native::toString(disconnectCode)));
    if (m_rtcscAppClient) {
        auto result = m_rtcscAppClient->disconnectSession(sessionId, disconnectCode);
        if (result != rtc::native::RtcscErrorCode::SUCCESS) {
            ACSDK_WARN(LX(__func__).d("reason", toString(result)));
        }
    }
}

bool LiveViewControllerCapabilityAgent::hasActiveLiveView() {
    return m_lastDisplayedDirective &&
           m_lastDisplayedDirective->directive->getNamespace() == LIVEVIEWCONTROLLER_CAPABILITY_INTERFACE_NAME &&
           m_lastDisplayedDirective->directive->getName() == START_LIVE_VIEW.name;
}

}  // namespace liveViewController
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
