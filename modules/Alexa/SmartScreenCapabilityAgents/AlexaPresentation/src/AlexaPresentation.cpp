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

#include <ostream>
#include <regex>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Metrics/MetricEventBuilder.h>
#include <AVSCommon/Utils/Metrics/DataPointStringBuilder.h>
#include <AVSCommon/Utils/Metrics/DataPointDurationBuilder.h>

#include "AlexaPresentation/AlexaPresentation.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

using namespace alexaClientSDK;
using namespace avsCommon::avs;
using namespace avsCommon::sdkInterfaces;
using namespace avsCommon::utils;
using namespace avsCommon::utils::configuration;
using namespace avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::timing;

/// AlexaPresentation capability constants
/// AlexaPresentation interface type
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";

/// AlexaPresentation interface name2
static const std::string ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_NAME = "Alexa.Presentation.APL";

/// AlexaPresentation interface version for Alexa.Presentation.APL
static const std::string ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_VERSION = "1.3";

/// AlexaPresentation interface name3
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_NAME = "Alexa.Presentation";

/// AlexaPresentation interface version for Alexa.Presentation.
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_VERSION = "1.0";

/// String to identify log entries originating from this file.
static const std::string TAG{"AlexaPresentation"};

/// The key in our config file to find the root of APL Presentation configuration.
static const std::string ALEXAPRESENTATION_CONFIGURATION_ROOT_KEY = "alexaPresentationCapabilityAgent";

/// The key in our config file to set the minimum time in ms between reporting proactive state report events
static const std::string ALEXAPRESENTATION_MIN_STATE_REPORT_INTERVAL_KEY = "minStateReportIntervalMs";

/// The key in our config file to set the time in ms between proactive state report checks - 0 disables the feature
static const std::string ALEXAPRESENTATION_STATE_REPORT_CHECK_INTERVAL_KEY = "stateReportCheckIntervalMs";

/// StaticRequestToken value for providing Change Report state
static const int PROACTIVE_STATE_REQUEST_TOKEN = 0;
/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The name of the @c FocusManager channel used by @c RenderingHandler.
static const std::string CHANNEL_NAME = avsCommon::sdkInterfaces::FocusManagerInterface::VISUAL_CHANNEL_NAME;

/// Namespace two supported by Alexa presentation capability agent.
static const std::string ALEXA_PRESENTATION_NAMESPACE{"Alexa.Presentation"};

/// Namespace three supported by Alexa presentation APL capability agent.
static const std::string ALEXA_PRESENTATION_APL_NAMESPACE{"Alexa.Presentation.APL"};

/// The name for RenderDocument directive.
static const std::string RENDER_DOCUMENT{"RenderDocument"};

/// The name for ExecuteCommand directive.
static const std::string EXECUTE_COMMAND{"ExecuteCommands"};

/// The name for SendIndexListData directive.
static const std::string SEND_INDEX_LIST_DATA{"SendIndexListData"};

/// The name for UpdateIndexListData directive.
static const std::string UPDATE_INDEX_LIST_DATA{"UpdateIndexListData"};

/// The name for SendTokenListData directive.
static const std::string SEND_TOKEN_LIST_DATA{"SendTokenListData"};

/// The name for UserEvent event.
static const std::string USER_EVENT{"UserEvent"};

/// The name for LoadIndexListData event.
static const std::string LOAD_INDEX_LIST_DATA{"LoadIndexListData"};

/// The name for LoadTokenListData event.
static const std::string LOAD_TOKEN_LIST_DATA{"LoadTokenListData"};

/// The name for RuntimeError event.
static const std::string RUNTIME_ERROR{"RuntimeError"};

/// The name for DocumentDismissed event.
static const std::string DOCUMENT_DISMISSED{"Dismissed"};

/// The RenderDocument directive signature.
static const NamespaceAndName DOCUMENT{ALEXA_PRESENTATION_APL_NAMESPACE, RENDER_DOCUMENT};

/// The ExecuteCommand directive signature.
static const NamespaceAndName COMMAND{ALEXA_PRESENTATION_APL_NAMESPACE, EXECUTE_COMMAND};

/// The SendIndexListData directive signature.
static const NamespaceAndName INDEX_LIST_DATA{ALEXA_PRESENTATION_APL_NAMESPACE, SEND_INDEX_LIST_DATA};

/// The UpdateIndexListData directive signature.
static const NamespaceAndName INDEX_LIST_UPDATE{ALEXA_PRESENTATION_APL_NAMESPACE, UPDATE_INDEX_LIST_DATA};

/// The SendTokenListData directive signature.
static const NamespaceAndName TOKEN_LIST_DATA{ALEXA_PRESENTATION_APL_NAMESPACE, SEND_TOKEN_LIST_DATA};

/// Name of the runtime configuration.
static const std::string RUNTIME_CONFIG = "runtime";

/// Identifier for the runtime (APL) version of the configuration.
static const std::string APL_MAX_VERSION = "maxVersion";

/// Identifier for the presentationToken's sent in a RenderDocument directive
static const std::string PRESENTATION_TOKEN = "presentationToken";

/// Identifier for the timeoutType sent in a RenderDocument directive
static const std::string TIMEOUTTYPE_FIELD = "timeoutType";

/// Identifier for the windowId's sent in a RenderDocument directive
static const std::string WINDOW_ID = "windowId";

/// Identifier for the document sent in a RenderDocument directive
static const std::string DOCUMENT_FIELD = "document";

/// Identifier for the commands sent in a RenderDocument directive
static const std::string COMMANDS_FIELD = "commands";

/// Tag for finding the visual context information sent from the runtime as part of event context.
static const std::string VISUAL_CONTEXT_NAME{"RenderedDocumentState"};

/// Dynamic index list data source type
static const std::string DYNAMIC_INDEX_LIST{"dynamicIndexList"};

/// Dynamic token list data source type
static const std::string DYNAMIC_TOKEN_LIST{"dynamicTokenList"};

/// The AlexaPresentation context state signature.
static const avsCommon::avs::NamespaceAndName RENDERED_DOCUMENT_STATE{
    ALEXA_PRESENTATION_APL_NAMESPACE,
    VISUAL_CONTEXT_NAME};

std::map<AlexaPresentation::MetricEvent, std::string> AlexaPresentation::MetricsDataPointNames = {
    {AlexaPresentation::MetricEvent::RENDER_DOCUMENT, "AlexaPresentation.RenderDocument.TimeTaken"},
    {AlexaPresentation::MetricEvent::LAYOUT, "View.Layout.TimeTaken"},
    {AlexaPresentation::MetricEvent::INFLATE, "APL.Inflate.TimeTaken"},
    {AlexaPresentation::MetricEvent::TEXT_MEASURE_COUNT, "APL.TextMeasurement.Count"},
    {AlexaPresentation::MetricEvent::DROP_FRAME, "View.DropFrame.Count"}};

static const std::string ACTIVITY_RENDER_DOCUMENT = "AlexaPresentation.renderDocument";
static const std::string ACTIVITY_RENDER_DOCUMENT_FAIL = "AlexaPresentation.renderDocument.fail";
static const std::string ACTIVITY_VIEW_LAYOUT = "AlexaPresentation.viewLayout";
static const std::string ACTIVITY_INFLATE_APL = "AlexaPresentation.inflateAPL";
static const std::string ACTIVITY_TEXT_MEASURE = "AlexaPresentation.textMeasure";
static const std::string ACTIVITY_DROP_FRAME = "AlexaPresentation.dropFrame";

/// Default minimum interval between state reports
static std::chrono::milliseconds DEFAULT_MIN_STATE_REPORT_INTERVAL_MS{600};

/// Default interval between proactive state report checks - disabled by default
static std::chrono::milliseconds DEFAULT_STATE_REPORT_CHECK_INTERVAL_MS{0};

/// Represents an invalid / unspecified timeout value
const std::chrono::milliseconds INVALID_TIMEOUT = std::chrono::milliseconds(-1);

std::shared_ptr<AlexaPresentation> AlexaPresentation::create(
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider,
    std::shared_ptr<avsCommon::sdkInterfaces::timing::TimerDelegateFactoryInterface> timerDelegateFactory) {
    if (!focusManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullFocusManager"));
        return nullptr;
    }

    if (!exceptionSender) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullExceptionSender"));
        return nullptr;
    }

    if (!messageSender) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMessageSender"));
        return nullptr;
    }

    if (!contextManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullContextManagerInterface"));
        return nullptr;
    }

    std::shared_ptr<AlexaPresentation> alexaPresentation(new AlexaPresentation(
        focusManager,
        exceptionSender,
        metricRecorder,
        messageSender,
        contextManager,
        visualStateProvider,
        timerDelegateFactory));

    if (!alexaPresentation->initialize()) {
        ACSDK_ERROR(LX("createFailed").d("reason", "Initialization error."));
        return nullptr;
    }
    if (visualStateProvider) {
        ACSDK_DEBUG3(LX(__func__).d("visualStateProvider", "On"));
        contextManager->setStateProvider(RENDERED_DOCUMENT_STATE, alexaPresentation);
    }

    return alexaPresentation;
}

/// Initializes the object by reading the values from configuration.
bool AlexaPresentation::initialize() {
    auto configurationRoot = ConfigurationNode::getRoot()[ALEXAPRESENTATION_CONFIGURATION_ROOT_KEY];

    configurationRoot.getDuration<std::chrono::milliseconds>(
        ALEXAPRESENTATION_MIN_STATE_REPORT_INTERVAL_KEY,
        &m_minStateReportInterval,
        DEFAULT_MIN_STATE_REPORT_INTERVAL_MS);

    configurationRoot.getDuration<std::chrono::milliseconds>(
        ALEXAPRESENTATION_STATE_REPORT_CHECK_INTERVAL_KEY,
        &m_stateReportCheckInterval,
        DEFAULT_STATE_REPORT_CHECK_INTERVAL_MS);

    if (m_stateReportCheckInterval.count() == 0) {
        ACSDK_DEBUG0(LX(__func__).m("Proactive state report timer disabled"));
    } else {
        if (m_stateReportCheckInterval < m_minStateReportInterval) {
            ACSDK_WARN(
                LX(__func__).m("State check interval cannot be less than minimum reporting interval, setting check "
                               "interval to minimum report interval"));
            m_stateReportCheckInterval = m_minStateReportInterval;
        }

        ACSDK_DEBUG0(LX(__func__)
                         .d("minStateReportIntervalMs", m_minStateReportInterval.count())
                         .d("stateReportCheckIntervalMs", m_stateReportCheckInterval.count()));

        m_proactiveStateTimer.start(
            m_stateReportCheckInterval,
            Timer::PeriodType::ABSOLUTE,
            Timer::FOREVER,
            std::bind(&AlexaPresentation::proactiveStateReport, this));
    }

    return true;
}

void AlexaPresentation::setExecutor(
    const std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor>& executor) {
    ACSDK_WARN(LX(__func__).d("reason", "should be called in test only"));
    m_executor = executor;
}

void AlexaPresentation::handleDirectiveImmediately(std::shared_ptr<AVSDirective> directive) {
    ACSDK_DEBUG5(LX(__func__));
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void AlexaPresentation::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));
    if (!info || !info->directive) {
        ACSDK_ERROR(LX("preHandleDirectiveFailed").d("reason", "nullDirectiveInfo"));
        return;
    }
}

void AlexaPresentation::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    // Must remain on the very fist line for accurate telemetry
    m_renderReceivedTime = std::chrono::steady_clock::now();

    ACSDK_DEBUG5(LX(__func__));
    if (!info || !info->directive) {
        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
        return;
    }

    if (info->directive->getNamespace() == DOCUMENT.nameSpace && info->directive->getName() == DOCUMENT.name) {
        handleRenderDocumentDirective(info);
    } else if (info->directive->getNamespace() == COMMAND.nameSpace && info->directive->getName() == COMMAND.name) {
        handleExecuteCommandDirective(info);
    } else if (
        info->directive->getNamespace() == INDEX_LIST_DATA.nameSpace &&
        info->directive->getName() == INDEX_LIST_DATA.name) {
        handleDynamicListDataDirective(info, DYNAMIC_INDEX_LIST);
    } else if (
        info->directive->getNamespace() == INDEX_LIST_UPDATE.nameSpace &&
        info->directive->getName() == INDEX_LIST_UPDATE.name) {
        handleDynamicListDataDirective(info, DYNAMIC_INDEX_LIST);
    } else if (
        info->directive->getNamespace() == TOKEN_LIST_DATA.nameSpace &&
        info->directive->getName() == TOKEN_LIST_DATA.name) {
        handleDynamicListDataDirective(info, DYNAMIC_TOKEN_LIST);
    } else {
        handleUnknownDirective(info);
    }
}

void AlexaPresentation::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

DirectiveHandlerConfiguration AlexaPresentation::getConfiguration() const {
    ACSDK_DEBUG5(LX(__func__));
    DirectiveHandlerConfiguration configuration;

    configuration[DOCUMENT] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, true);
    configuration[COMMAND] = BlockingPolicy(BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[INDEX_LIST_DATA] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, false);
    configuration[INDEX_LIST_UPDATE] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, false);
    configuration[TOKEN_LIST_DATA] = BlockingPolicy(BlockingPolicy::MEDIUM_VISUAL, false);
    return configuration;
}

void AlexaPresentation::clearCard() {
    ACSDK_DEBUG5(LX(__func__));
    m_executor->submit([this]() {
        ACSDK_DEBUG5(LX("clearCardExecutor"));
        executeResetActivityTracker();
        executeClearCardEvent();
        // TODO: ARC-575 consider cleaning the commands on CommandsSequencer
        executeClearExecuteCommands("Card cleared");
    });
}

void AlexaPresentation::clearExecuteCommands(const std::string& token, const bool markAsFailed) {
    m_executor->submit(
        [this, token, markAsFailed]() { executeClearExecuteCommands("User exited", token, markAsFailed); });
}

void AlexaPresentation::executeClearExecuteCommands(
    const std::string& reason,
    const std::string& token,
    const bool markAsFailed) {
    ACSDK_DEBUG5(LX(__func__));
    if (!(m_lastExecuteCommandTokenAndDirective.first.empty()) && m_lastExecuteCommandTokenAndDirective.second &&
        m_lastExecuteCommandTokenAndDirective.second->result) {
        if (!token.empty() && m_lastExecuteCommandTokenAndDirective.first != token) {
            ACSDK_ERROR(LX(__func__).d(
                "reason", "presentationToken in the last ExecuteCommand does not match with the provided token."));
            return;
        }
        if (markAsFailed) {
            m_lastExecuteCommandTokenAndDirective.second->result->setFailed(reason);
        } else {
            m_lastExecuteCommandTokenAndDirective.second->result->setCompleted();
        }
    }

    m_lastExecuteCommandTokenAndDirective.first.clear();
}

void AlexaPresentation::onFocusChanged(
    avsCommon::avs::FocusState newFocus,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    m_executor->submit([this, newFocus]() { executeOnFocusChangedEvent(newFocus); });
}

void AlexaPresentation::onDialogUXStateChanged(
    avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState newState) {
    ACSDK_DEBUG5(LX(__func__).d("state", newState));
    m_executor->submit([this, newState]() {
        m_dialogUxState = newState;
        if (avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE == newState &&
            smartScreenSDKInterfaces::State::DISPLAYING == m_state) {
            // Restart timer in case if event arrived while GUI is not active.
            if (m_lastDisplayedDirective && m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT &&
                InteractionState::INACTIVE == m_documentInteractionState) {
                executeStartOrExtendTimer();
            }
        } else {
            if (m_lastDisplayedDirective && m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT) {
                executeStopTimer();
            }
        }
    });
}

void AlexaPresentation::addObserver(
    std::shared_ptr<smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer) {
    ACSDK_DEBUG5(LX(__func__));
    if (!observer) {
        ACSDK_ERROR(LX("addObserverFailed").d("reason", "Observer is null."));
        return;
    }
    m_executor->submit([this, observer]() {
        ACSDK_DEBUG5(LX("addObserverInExecutor"));
        if (m_observers.insert(observer).second) {
            observer->onMetricRecorderAvailable(m_metricRecorder);
        } else {
            ACSDK_ERROR(LX("addObserverFailedInExecutor").d("reason", "Duplicate observer."));
        }
    });
}

void AlexaPresentation::removeObserver(
    std::shared_ptr<smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer) {
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

AlexaPresentation::AlexaPresentation(
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider,
    std::shared_ptr<avsCommon::sdkInterfaces::timing::TimerDelegateFactoryInterface> timerDelegateFactory) :
        CapabilityAgent{ALEXA_PRESENTATION_NAMESPACE, exceptionSender},
        RequiresShutdown{"AlexaPresentation"},
        m_idleTimer{timerDelegateFactory},
        m_focus{FocusState::NONE},
        m_state{smartScreenSDKInterfaces::State::IDLE},
        m_dialogUxState{DialogUXState::IDLE},
        m_focusManager{focusManager},
        m_messageSender{messageSender},
        m_contextManager{contextManager},
        m_visualStateProvider{visualStateProvider},
        m_APLVersion{},
        m_documentInteractionState{AlexaPresentation::InteractionState::INACTIVE},
        m_metricRecorder{metricRecorder},
        m_lastReportTime{std::chrono::steady_clock::now()},
        m_minStateReportInterval{DEFAULT_MIN_STATE_REPORT_INTERVAL_MS},
        m_stateReportPending{false},
        m_documentRendered{false},
        m_presentationSession{} {
    m_executor = std::make_shared<alexaClientSDK::avsCommon::utils::threading::Executor>();
    m_capabilityConfigurations.insert(getAlexaPresentationCapabilityConfiguration());
}

std::shared_ptr<CapabilityConfiguration> AlexaPresentation::getAlexaPresentationAPLCapabilityConfiguration() {
    if (m_APLVersion.empty()) {
        ACSDK_ERROR(LX("getAlexaPresentationAPLCapabilityConfigurationFailed").d("reason", "empty APL Version"));
        return nullptr;
    }

    std::unordered_map<std::string, std::string> configMap;

    configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, ALEXAPRESENTATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_NAME});
    configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_VERSION});

    rapidjson::Document runtime(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& alloc = runtime.GetAllocator();

    rapidjson::Value configJson;
    configJson.SetObject();

    configJson.AddMember(rapidjson::StringRef(APL_MAX_VERSION), m_APLVersion, alloc);
    runtime.AddMember(rapidjson::StringRef(RUNTIME_CONFIG), configJson, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!runtime.Accept(writer)) {
        ACSDK_CRITICAL(
            LX("getAlexaPresentationAPLCapabilityConfigurationFailed").d("reason", "configWriterRefusedJsonObject"));
        return nullptr;
    }

    configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, buffer.GetString()});

    return std::make_shared<CapabilityConfiguration>(configMap);
}

std::shared_ptr<CapabilityConfiguration> AlexaPresentation::getAlexaPresentationCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;

    configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, ALEXAPRESENTATION_CAPABILITY_INTERFACE_TYPE});
    configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXAPRESENTATION_CAPABILITY_INTERFACE_NAME});
    configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALEXAPRESENTATION_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<CapabilityConfiguration>(configMap);
}

void AlexaPresentation::sendUserEvent(const std::string& payload) {
    m_executor->submit([this, payload]() { executeSendEvent(ALEXA_PRESENTATION_APL_NAMESPACE, USER_EVENT, payload); });
}

void AlexaPresentation::sendDataSourceFetchRequestEvent(const std::string& type, const std::string& payload) {
    if (type == DYNAMIC_INDEX_LIST) {
        m_executor->submit(
            [this, payload]() { executeSendEvent(ALEXA_PRESENTATION_APL_NAMESPACE, LOAD_INDEX_LIST_DATA, payload); });
    } else if (type == DYNAMIC_TOKEN_LIST) {
        m_executor->submit(
            [this, payload]() { executeSendEvent(ALEXA_PRESENTATION_APL_NAMESPACE, LOAD_TOKEN_LIST_DATA, payload); });
    } else {
        ACSDK_WARN(LX("sendDataSourceFetchRequestEventIgnored").d("reason", "Trying to process unknown data source."));
        return;
    }
}

void AlexaPresentation::sendRuntimeErrorEvent(const std::string& payload) {
    m_executor->submit(
        [this, payload]() { executeSendEvent(ALEXA_PRESENTATION_APL_NAMESPACE, RUNTIME_ERROR, payload); });
}

void AlexaPresentation::doShutdown() {
    m_proactiveStateTimer.stop();
    m_executor->shutdown();

    executeClearExecuteCommands("AlexaPresentationShuttingDown");

    m_visualStateProvider.reset();
    m_messageSender.reset();
    m_contextManager.reset();
    m_focusManager.reset();
    m_observers.clear();
}

void AlexaPresentation::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if (info->directive && info->result) {
        CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void AlexaPresentation::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info) {
        if (info->result) {
            info->result->setCompleted();
        }
        removeDirective(info);
    }
}

bool AlexaPresentation::parseDirectivePayload(std::shared_ptr<DirectiveInfo> info, rapidjson::Document* document) {
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

void AlexaPresentation::handleRenderDocumentDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));

    m_executor->submit([this, info]() {
        ACSDK_DEBUG9(LX("handleRenderDocumentDirectiveInExecutor").sensitive("payload", info->directive->getPayload()));
        rapidjson::Document payload;
        if (!parseDirectivePayload(info, &payload)) {
            notifyAbort();
            return;
        }

        std::string presentationToken;
        if (!jsonUtils::retrieveValue(payload, PRESENTATION_TOKEN, &presentationToken)) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor").d("reason", "NoPresentationToken"));
            sendExceptionEncounteredAndReportFailed(info, "missing presentationToken");
            notifyAbort();
            return;
        }

        std::string timeoutType;
        if (!jsonUtils::retrieveValue(payload, TIMEOUTTYPE_FIELD, &timeoutType)) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor").d("reason", "NoTimeoutTypeField"));
            sendExceptionEncounteredAndReportFailed(info, "missing timeoutType");
            notifyAbort();
            return;
        }

        // Validate timeoutType
        auto maybeValidTimeout = TimeoutTypeUtils::fromString(timeoutType);
        if (!maybeValidTimeout.hasValue()) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor")
                            .d("reason", "InvalidTimeoutType")
                            .d("receivedTimeoutType", timeoutType));
            sendExceptionEncounteredAndReportFailed(info, "invalid timeoutType");
            notifyAbort();
            return;
        }
        m_documentInteractionTimeout = TimeoutTypeUtils::asDuration(maybeValidTimeout.value());

        std::string APLdocument;
        if (!jsonUtils::retrieveValue(payload, DOCUMENT_FIELD, &APLdocument)) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor").d("reason", "NoDocument"));
            sendExceptionEncounteredAndReportFailed(info, "missing APLdocument");
            notifyAbort();
            return;
        }

        PresentationSession presentationSession = {};
        rapidjson::Value::ConstMemberIterator iterator;
        std::string presentationSessionPayload;
        if (jsonUtils::findNode(payload, PRESENTATION_SESSION_FIELD, &iterator) &&
            jsonUtils::convertToValue(iterator->value, &presentationSessionPayload)) {
            std::string skillId;
            if (!jsonUtils::retrieveValue(presentationSessionPayload, SKILL_ID, &skillId)) {
                ACSDK_WARN(
                    LX("handleRenderDocumentDirectiveInExecutor").m("Failed to find presentationSession skillId"));
            }

            std::string id;
            if (!jsonUtils::retrieveValue(presentationSessionPayload, PRESENTATION_SESSION_ID, &id)) {
                ACSDK_WARN(LX("handleRenderDocumentDirectiveInExecutor").m("Failed to find presentationSession id"));
            }

            rapidjson::Document doc;
            doc.Parse(presentationSessionPayload);
            std::vector<smartScreenSDKInterfaces::GrantedExtension> grantedExtensions;
            if (doc.HasMember(PRESENTATION_SESSION_GRANTEDEXTENSIONS) &&
                doc[PRESENTATION_SESSION_GRANTEDEXTENSIONS].IsArray()) {
                auto grantExtensionArray = doc[PRESENTATION_SESSION_GRANTEDEXTENSIONS].GetArray();
                for (auto& itr : grantExtensionArray) {
                    if (itr.HasMember(PRESENTATION_SESSION_URI) && itr[PRESENTATION_SESSION_URI].IsString()) {
                        auto grantedExtension = smartScreenSDKInterfaces::GrantedExtension();
                        grantedExtension.uri = itr[PRESENTATION_SESSION_URI].GetString();
                        grantedExtensions.push_back(std::move(grantedExtension));
                    } else {
                        ACSDK_WARN(LX("handleRenderDocumentDirectiveInExecutor").m("Error parsing grantedExtensions"));
                    }
                }
            } else {
                ACSDK_WARN(LX("handleRenderDocumentDirectiveInExecutor")
                               .m("Failed to find presentationSession grantedExtensions"));
            }

            std::vector<smartScreenSDKInterfaces::AutoInitializedExtension> autoInitializedExtensions;
            if (doc.HasMember(PRESENTATION_SESSION_AUTOINITIALIZEDEXTENSIONS) &&
                doc[PRESENTATION_SESSION_AUTOINITIALIZEDEXTENSIONS].IsArray()) {
                auto autoInitializedExtensionArray = doc[PRESENTATION_SESSION_AUTOINITIALIZEDEXTENSIONS].GetArray();
                for (auto& itr : autoInitializedExtensionArray) {
                    if (itr.HasMember(PRESENTATION_SESSION_URI) && itr[PRESENTATION_SESSION_URI].IsString() &&
                        itr.HasMember(PRESENTATION_SESSION_SETTINGS) && itr[PRESENTATION_SESSION_SETTINGS].IsString()) {
                        auto autoInitializedExtension = smartScreenSDKInterfaces::AutoInitializedExtension();
                        autoInitializedExtension.uri = itr[PRESENTATION_SESSION_URI].GetString();
                        autoInitializedExtension.settings = itr[PRESENTATION_SESSION_SETTINGS].GetString();
                        autoInitializedExtensions.push_back(std::move(autoInitializedExtension));
                    } else {
                        ACSDK_WARN(
                            LX("handleRenderDocumentDirectiveInExecutor").m("Error parsing autoInitializedExtensions"));
                    }
                }
            } else {
                ACSDK_WARN(LX("handleRenderDocumentDirectiveInExecutor")
                               .m("Failed to find presentationSession autoInitializedExtensions"));
            }

            presentationSession = PresentationSession(skillId, id, grantedExtensions, autoInitializedExtensions);
        }

        if (presentationSession != m_presentationSession) {
            ACSDK_DEBUG0(LX("handleRenderDocumentDirectiveInExecutor")
                             .m("PresentationSessionChanged")
                             .d("previousSkillId", m_presentationSession.skillId)
                             .d("newSkillId", presentationSession.skillId));
            for (auto& observer : m_observers) {
                observer->onPresentationSessionChanged(
                    presentationSession.id,
                    presentationSession.skillId,
                    presentationSession.grantedExtensions,
                    presentationSession.autoInitializedExtensions);
            }
            m_presentationSession = presentationSession;
        }

        executeRenderDocumentEvent(info);
    });
}

void AlexaPresentation::handleExecuteCommandDirective(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));

    m_executor->submit([this, info]() {
        ACSDK_DEBUG5(LX("handleExecuteCommandDirectiveInExecutor"));
        rapidjson::Document payload;
        if (!parseDirectivePayload(info, &payload)) {
            return;
        }

        std::string presentationToken;
        if (!jsonUtils::retrieveValue(payload, PRESENTATION_TOKEN, &presentationToken)) {
            ACSDK_ERROR(LX("handleExecuteCommandDirectiveFailedInExecutor")
                            .d("reason", "No presentationToken in the ExecuteCommand directive."));
            sendExceptionEncounteredAndReportFailed(info, "missing presentationToken");
            return;
        }

        if (!jsonUtils::jsonArrayExists(payload, COMMANDS_FIELD)) {
            ACSDK_ERROR(LX("handleExecuteCommandDirectiveFailedInExecutor")
                            .d("reason", "No command array in the ExecuteCommand directive."));
            sendExceptionEncounteredAndReportFailed(info, "missing commands");
            return;
        }

        if (!m_lastDisplayedDirective) {
            ACSDK_ERROR(LX("handleExecuteCommandDirectiveFailedInExecutor")
                            .d("reason", "No display directive before call to ExecuteCommand."));
            sendExceptionEncounteredAndReportFailed(info, "missing previous rendering directive");
            return;
        }

        rapidjson::Document renderedPayload;
        if (!parseDirectivePayload(m_lastDisplayedDirective, &renderedPayload)) {
            sendExceptionEncounteredAndReportFailed(info, "Parse error of previous render directive");
            ACSDK_ERROR(LX("handleExecuteCommandDirectiveFailedInExecutor")
                            .d("reason", "Could not parse the last displayed directive."));

            return;
        }

        std::string renderedPresentationToken;
        if (!jsonUtils::retrieveValue(renderedPayload, PRESENTATION_TOKEN, &renderedPresentationToken)) {
            sendExceptionEncounteredAndReportFailed(info, "Missing presentationToken in last display directive.");
            ACSDK_ERROR(LX("handleExecuteCommandDirectiveFailedInExecutor")
                            .d("reason", "No presentationToken in the last displayed directive."));
            return;
        }

        if (presentationToken != renderedPresentationToken) {
            sendExceptionEncounteredAndReportFailed(
                info, "token mismatch between ExecuteCommand and last rendering directive.");
            ACSDK_ERROR(
                LX("handleExecuteCommandDirectiveFailedInExecutor")
                    .d("reason",
                       "presentationToken in executeCommand does not match the one from last displayed directive."));
            return;
        }

        m_lastExecuteCommandTokenAndDirective = std::make_pair(presentationToken, info);
        executeExecuteCommandEvent(info);
    });
}

void AlexaPresentation::handleDynamicListDataDirective(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& sourceType) {
    ACSDK_DEBUG5(LX(__func__));

    m_executor->submit([this, info, sourceType]() {
        ACSDK_DEBUG9(
            LX("handleDynamicListDataDirectiveInExecutor").sensitive("payload", info->directive->getPayload()));
        rapidjson::Document payload;
        if (!parseDirectivePayload(info, &payload)) {
            return;
        }

        std::string presentationToken;
        if (!jsonUtils::retrieveValue(payload, PRESENTATION_TOKEN, &presentationToken)) {
            ACSDK_ERROR(LX("handleDynamicListDataDirectiveFailedInExecutor").d("reason", "NoPresentationToken"));
            sendExceptionEncounteredAndReportFailed(info, "missing presentationToken");
            return;
        }

        if (!m_lastDisplayedDirective) {
            ACSDK_ERROR(LX("handleDynamicListDataDirectiveFailedInExecutor")
                            .d("reason", "No display directive before call to DynamicListData directive."));
            sendExceptionEncounteredAndReportFailed(info, "missing previous rendering directive");
            return;
        }

        rapidjson::Document renderedPayload;
        if (!parseDirectivePayload(m_lastDisplayedDirective, &renderedPayload)) {
            sendExceptionEncounteredAndReportFailed(info, "Parse error of previous render directive");
            ACSDK_ERROR(LX("handleDynamicListDataDirectiveFailedInExecutor")
                            .d("reason", "Could not parse the last displayed directive."));

            return;
        }

        std::string renderedPresentationToken;
        if (!jsonUtils::retrieveValue(renderedPayload, PRESENTATION_TOKEN, &renderedPresentationToken)) {
            sendExceptionEncounteredAndReportFailed(info, "Missing presentationToken in last display directive.");
            ACSDK_ERROR(LX("handleDynamicListDataDirectiveFailedInExecutor")
                            .d("reason", "No presentationToken in the last displayed directive."));
            return;
        }

        if (presentationToken != renderedPresentationToken) {
            sendExceptionEncounteredAndReportFailed(
                info, "token mismatch between DynamicListData and last rendering directive.");
            ACSDK_ERROR(
                LX("handleDynamicListDataDirectiveFailedInExecutor")
                    .d("reason",
                       "presentationToken in DynamicListData does not match the one from last displayed directive."));
            return;
        }

        // Core will do checks for us for content of it, so just pass through.
        executeDataSourceUpdateEvent(info, sourceType);
    });
}

void AlexaPresentation::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
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

/**
 * Get the token from payload of renderDocument message with APL document
 *
 * @param payload - the payload of the message
 * @return The token for APL payload, empty string otherwise.
 */
static const std::string getAPLToken(const std::string& payload) {
    rapidjson::Document document;
    document.Parse(payload);

    std::string APLToken;
    if (!jsonUtils::retrieveValue(document, PRESENTATION_TOKEN, &APLToken)) {
        ACSDK_ERROR(LX("getAPLTokenFailed").d("reason", "Couldn't find token in APL document"));
        return "";
    }
    ACSDK_DEBUG5(LX(__func__).d("Token", APLToken));

    return APLToken;
}

/**
 * Get the target windowId from payload of renderDocument message with APL document
 *
 * @param payload - the payload of the message
 * @return The windowId for APL payload, empty string otherwise.
 */
static const std::string getTargetWindowId(const std::string& payload) {
    rapidjson::Document document;
    document.Parse(payload);

    std::string targetWindowId;
    if (!jsonUtils::retrieveValue(document, WINDOW_ID, &targetWindowId)) {
        ACSDK_ERROR(LX("getTargetWindowIdFailed").d("reason", "Couldn't find windowId in APL document"));
        return "";
    }
    ACSDK_DEBUG5(LX(__func__).d("Target Window Id", targetWindowId));

    return targetWindowId;
}

void AlexaPresentation::executeRenderDocumentCallbacks(bool isClearCard) {
    bool dismissPrevious = !m_lastRenderedAPLToken.empty();
    std::string newToken;
    std::string windowId;

    if (!isClearCard) {
        newToken = getAPLToken(m_lastDisplayedDirective->directive->getPayload());
        windowId = getTargetWindowId(m_lastDisplayedDirective->directive->getPayload());
    }

    ACSDK_DEBUG3(LX(__func__)
                     .d("previousToken", m_lastRenderedAPLToken)
                     .d("newToken", newToken)
                     .d("isClear", isClearCard)
                     .d("windowId", windowId));

    m_documentRendered = false;
    startMetricsEvent(MetricEvent::RENDER_DOCUMENT);

    for (auto& observer : m_observers) {
        if (isClearCard) {
            m_presentationSession = {};
            observer->clearDocument(m_lastRenderedAPLToken, true);
        } else {
            if (dismissPrevious && m_lastTargetedWindowId != windowId) {
                observer->clearDocument(m_lastRenderedAPLToken, false);
            }
            observer->renderDocument(m_lastDisplayedDirective->directive->getPayload(), newToken, windowId);
            if (m_renderReceivedTime.time_since_epoch().count() != 0) {
                observer->onRenderDirectiveReceived(m_lastRenderedAPLToken, m_renderReceivedTime);
                m_renderReceivedTime = {};
            }
        }
    }

    if (dismissPrevious) {
        /*
         * Send @c Dismissed event for the previous document
         * Whether we are displaying new card or just dismissing this one.
         */
        ACSDK_DEBUG5(LX(__func__).d("Token", m_lastRenderedAPLToken));

        // Assemble the event payload.
        std::ostringstream payload;
        payload << R"({"presentationToken":")" << m_lastRenderedAPLToken << R"("})";

        executeSendEvent(ALEXA_PRESENTATION_NAMESPACE, DOCUMENT_DISMISSED, payload.str());
    }

    m_lastTargetedWindowId = windowId;
    m_lastRenderedAPLToken = newToken;
}

void AlexaPresentation::executeRenderDocument() {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective &&
        m_lastDisplayedDirective->directive->getNamespace() == ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT) {
        executeResetActivityTracker();
        executeRenderDocumentCallbacks(false);
    }
}

void AlexaPresentation::executeExecuteCommand(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective->directive->getNamespace() != ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() != RENDER_DOCUMENT) {
        sendExceptionEncounteredAndReportFailed(info, "APL document that requires command executed is not rendered.");
        ACSDK_ERROR(LX("executeExecuteCommandFailed")
                        .d("reason", "Cannot execute command when an APL document is not rendered."));
        return;
    }

    std::string presentationToken = getAPLToken(info->directive->getPayload());

    for (auto& observer : m_observers) {
        observer->executeCommands(info->directive->getPayload(), presentationToken);
    }
}

void AlexaPresentation::executeDataSourceUpdate(std::shared_ptr<DirectiveInfo> info, const std::string& sourceType) {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective->directive->getNamespace() != ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() != RENDER_DOCUMENT) {
        sendExceptionEncounteredAndReportFailed(info, "APL document that requires data source update is not rendered.");
        ACSDK_ERROR(LX("executeDataSourceUpdateFailed")
                        .d("reason", "Cannot do DataSource update when an APL document is not rendered."));
        return;
    }

    std::string presentationToken = getAPLToken(info->directive->getPayload());

    for (auto& observer : m_observers) {
        observer->dataSourceUpdate(sourceType, info->directive->getPayload(), presentationToken);
    }

    setHandlingCompleted(info);
}

void AlexaPresentation::executeClearCard() {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective &&
        m_lastDisplayedDirective->directive->getNamespace() == ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT) {
        executeRenderDocumentCallbacks(true);
    }
}

void AlexaPresentation::executeStartOrExtendTimer() {
    if (smartScreenSDKInterfaces::State::DISPLAYING == m_state) {
        m_idleTimer.stop();

        ACSDK_DEBUG3(
            LX(__func__)
                .d("timeoutInMilliseconds.hasValue", m_documentInteractionTimeout.hasValue())
                .d("timeoutinMilliseconds.value", m_documentInteractionTimeout.valueOr(INVALID_TIMEOUT).count()));
        if (m_documentInteractionTimeout.hasValue()) {
            m_idleTimer.start(m_documentInteractionTimeout.value(), [this] {
                m_executor->submit([this] { executeClearCardEvent(); });
            });
        }
    }
}

void AlexaPresentation::executeStopTimer() {
    ACSDK_DEBUG5(LX(__func__));
    m_delayedExecutionTimer.stop();
    m_idleTimer.stop();
}

/*
 * A state machine is used to acquire and release the visual channel from the visual @c FocusManager.  The state machine
 * has five @c State, and four events as listed below:
 *
 * renderDocument - This event happens when the AlexaPresentation is ready to notify its observers to display a
 * document.
 *
 * focusChanged - This event happens when the @c FocusManager notifies a change in @c FocusState in the visual
 * channel.
 *
 * timer - This event happens when m_idleTimer expires and needs to notify its observers to clear the
 * document.
 *
 * cardCleared - This event happens when @c displayCardCleared() is called to notify @c RenderingHandler the device has
 * cleared the screen.
 *
 * Each state transition may result in one or more of the following actions:
 * (A) Acquire channel
 * (B) Release channel
 * (C) Notify observers to display document
 * (D) Notify observers to clear document
 * (E) Log error about unexpected focusChanged event.
 *
 * Below is the state table illustrating the state transition and its action.  NC means no change in state.
 *
 *                                              E  V  E  N  T  S
 *                -----------------------------------------------------------------------------------------
 *  Current State | render       | timer          | focusChanged::NONE | focusChanged::FG/BG | cardCleared
 * --------------------------------------------------------------------------------------------------------
 * | IDLE         | ACQUIRING(A) | NC             | NC                 | RELEASING(B&E)      | NC
 * | ACQUIRING    | NC           | NC             | IDLE(E)            | DISPLAYING(C)       | NC
 * | DISPLAYING   | NC(C)        | RELEASING(B&D) | IDLE(D)            | DISPLAYING(C)       | RELEASING(B)
 * | RELEASING    | REACQUIRING  | NC             | IDLE               | NC(B&E)             | NC
 * | REACQUIRING  | NC           | NC             | ACQUIRING(A)       | RELEASING(B&E)      | NC
 * --------------------------------------------------------------------------------------------------------
 *
 */

void AlexaPresentation::executeClearCardEvent() {
    smartScreenSDKInterfaces::State nextState = m_state;

    switch (m_state) {
        case smartScreenSDKInterfaces::State::DISPLAYING:
            executeClearCard();
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
    ACSDK_DEBUG3(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void AlexaPresentation::executeOnFocusChangedEvent(avsCommon::avs::FocusState newFocus) {
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
                    executeRenderDocument();
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
                    executeRenderDocument();
                    break;
                case FocusState::NONE:
                    executeClearCard();
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
                    m_focusHoldingInterface = m_lastDisplayedDirective->directive->getNamespace();
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
    ACSDK_DEBUG3(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void AlexaPresentation::executeRenderDocumentEvent(
    const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info) {
    smartScreenSDKInterfaces::State nextState = m_state;
    m_lastDisplayedDirective = info;

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
                executeRenderDocument();
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
    ACSDK_DEBUG3(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void AlexaPresentation::executeExecuteCommandEvent(
    const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info) {
    smartScreenSDKInterfaces::State nextState = m_state;

    switch (m_state) {
        case smartScreenSDKInterfaces::State::IDLE:
        case smartScreenSDKInterfaces::State::ACQUIRING:
            // Do Nothing.
            break;
        case smartScreenSDKInterfaces::State::DISPLAYING:
            executeExecuteCommand(info);
            nextState = smartScreenSDKInterfaces::State::DISPLAYING;
            break;
        case smartScreenSDKInterfaces::State::RELEASING:
            nextState = smartScreenSDKInterfaces::State::REACQUIRING;
            break;
        case smartScreenSDKInterfaces::State::REACQUIRING:
            // Do Nothing.
            break;
    }
    ACSDK_DEBUG3(LX(__func__)
                     .d("prevState", smartScreenSDKInterfaces::stateToString(m_state))
                     .d("nextState", smartScreenSDKInterfaces::stateToString(nextState)));
    m_state = nextState;
}

void AlexaPresentation::executeDataSourceUpdateEvent(
    const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info,
    const std::string& sourceType) {
    switch (m_state) {
        case smartScreenSDKInterfaces::State::DISPLAYING:
            executeDataSourceUpdate(info, sourceType);
            break;
        default:
            // Do nothing
            break;
    }
}

std::unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> AlexaPresentation::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void AlexaPresentation::executeSendEvent(
    const std::string& avsNamespace,
    const std::string& name,
    const std::string& payload) {
    m_events.push(std::make_tuple(avsNamespace, name, payload));
    m_contextManager->getContext(shared_from_this());
}

void AlexaPresentation::onContextAvailable(const std::string& jsonContext) {
    auto task = [this, jsonContext]() {
        ACSDK_DEBUG9(LX("onContextAvailableExecutor"));

        if (!m_events.empty()) {
            auto event = m_events.front();
            auto msgIdAndJsonEvent = avsCommon::avs::buildJsonEventString(
                std::get<0>(event), std::get<1>(event), "", std::get<2>(event), jsonContext);
            auto userEventMessage = std::make_shared<avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);
            ACSDK_DEBUG9(LX("Sending event to AVS").d("namespace", std::get<0>(event)).d("name", std::get<1>(event)));
            m_messageSender->sendMessage(userEventMessage);

            m_events.pop();
        }
    };

    m_executor->submit(task);
}

void AlexaPresentation::onContextFailure(const ContextRequestError error) {
    ACSDK_ERROR(LX(__func__).d("reason", "contextRequestErrorOccurred").d("error", error));
}

void AlexaPresentation::provideState(
    const avsCommon::avs::NamespaceAndName& stateProviderName,
    unsigned int stateRequestToken) {
    m_executor->submit([this, stateRequestToken]() { executeProvideState(stateRequestToken); });
}

void AlexaPresentation::executeProvideState(unsigned int stateRequestToken) {
    ACSDK_DEBUG3(LX(__func__).d("token", stateRequestToken));

    if (!m_visualStateProvider) {
        ACSDK_ERROR(LX("executeProvideStateFailed").d("reason", "no visualStateProvider"));
        return;
    }

    if (m_lastDisplayedDirective && !m_lastRenderedAPLToken.empty() &&
        ALEXA_PRESENTATION_APL_NAMESPACE == m_lastDisplayedDirective->directive->getNamespace()) {
        m_visualStateProvider->provideState(m_lastRenderedAPLToken, stateRequestToken);
    } else {
        m_contextManager->setState(RENDERED_DOCUMENT_STATE, "", StateRefreshPolicy::SOMETIMES, stateRequestToken);
        m_lastReportedState.clear();
    }
}

void AlexaPresentation::onVisualContextAvailable(const unsigned int requestToken, const std::string& visualContext) {
    ACSDK_DEBUG3(LX(__func__).d("requestToken", requestToken).sensitive("visualContext", visualContext));
    m_executor->submit([this, requestToken, visualContext]() {
        ACSDK_DEBUG3(LX("onVisualContextAvailableExecutor"));

        rapidjson::Document doc;
        std::string payload;

        // If valid visualContext, add presentationSession to payload
        if (!doc.Parse(visualContext).HasParseError()) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            m_presentationSession.addPresentationSessionPayload(&doc);
            doc.Accept(writer);
            payload = buffer.GetString();
        } else {
            // Otherwise make presentationSession the payload.
            payload = m_presentationSession.getPresentationSessionPayload();
        }

        CapabilityState state(payload);
        m_lastReportTime = std::chrono::steady_clock::now();
        m_stateReportPending = false;
        if (PROACTIVE_STATE_REQUEST_TOKEN == requestToken) {
            // Proactive visualContext report
            if (m_lastReportedState != visualContext) {
                m_contextManager->reportStateChange(
                    RENDERED_DOCUMENT_STATE, state, AlexaStateChangeCauseType::ALEXA_INTERACTION);
                m_lastReportedState = visualContext;
            }
        } else {
            if (m_lastDisplayedDirective && !m_lastRenderedAPLToken.empty() &&
                ALEXA_PRESENTATION_APL_NAMESPACE == m_lastDisplayedDirective->directive->getNamespace()) {
                m_contextManager->provideStateResponse(RENDERED_DOCUMENT_STATE, state, requestToken);
            } else {
                // Since requesting the visualContext, APL is no longer being displayed
                // Set presentationSession as the state.
                m_contextManager->setState(
                    RENDERED_DOCUMENT_STATE,
                    m_presentationSession.getPresentationSessionPayload(),
                    StateRefreshPolicy::SOMETIMES,
                    requestToken);
                m_lastReportedState.clear();
            }
        }
    });
}

void AlexaPresentation::setAPLMaxVersion(const std::string& APLMaxVersion) {
    ACSDK_DEBUG1(LX(__func__).d("APLVersion", APLMaxVersion));

    if (APLMaxVersion.empty()) {
        return;
    }

    m_APLVersion = APLMaxVersion;
    m_capabilityConfigurations.insert(getAlexaPresentationAPLCapabilityConfiguration());
}

void AlexaPresentation::setDocumentIdleTimeout(std::chrono::milliseconds timeout) {
    ACSDK_DEBUG1(LX(__func__).d("timeout", timeout.count()));

    if (INVALID_TIMEOUT != timeout) {
        m_documentInteractionTimeout.set(timeout);
    }
}

void AlexaPresentation::processRenderDocumentResult(
    const std::string& token,
    const bool result,
    const std::string& error) {
    m_executor->submit([this, token, result, error]() {
        if (token.empty()) {
            ACSDK_WARN(LX("processRenderDocumentResultFailedInExecutor").d("reason", "token is empty"));
            return;
        }

        ACSDK_DEBUG3(LX("processRenderDocumentResultExecutor").d("token", token).d("result", result));

        if (token == getNonAPLDocumentToken()) {
            // There is no need to perform further checks if this document is not APL
            return;
        }

        if (m_lastRenderedAPLToken != token) {
            ACSDK_ERROR(LX("processRenderDocumentResultFailedInExecutor")
                            .d("reason", "tokenMismatch")
                            .d("expected", m_lastRenderedAPLToken)
                            .d("actual", token));
            return;
        }

        if (result) {
            setHandlingCompleted(m_lastDisplayedDirective);
            executeProactiveStateReport();
        } else {
            sendExceptionEncounteredAndReportFailed(m_lastDisplayedDirective, "Renderer failed: " + error);
            resetMetricsEvent(MetricEvent::RENDER_DOCUMENT);
            endMetricsEvent(MetricEvent::RENDER_DOCUMENT, ACTIVITY_RENDER_DOCUMENT_FAIL);
            notifyAbort();
        }

        if (DialogUXState::IDLE == m_dialogUxState && InteractionState::INACTIVE == m_documentInteractionState) {
            executeStartOrExtendTimer();
        }
    });
}

void AlexaPresentation::processExecuteCommandsResult(
    const std::string& token,
    const bool result,
    const std::string& error) {
    m_executor->submit([this, token, result, error]() {
        ACSDK_DEBUG3(LX("processExecuteCommandsResultExecutor").d("token", token).d("result", result));

        bool isSuccess = result;
        if (token.empty()) {
            ACSDK_ERROR(LX("processExecuteCommandsResultExecutorFailed").d("reason", "token is empty"));
            isSuccess = false;
        } else if (token != m_lastExecuteCommandTokenAndDirective.first) {
            ACSDK_ERROR(LX("processExecuteCommandsResultExecutorFailed")
                            .d("reason", "asked to process missing directive")
                            .d("messageId", token));
            isSuccess = false;
        } else if (!m_lastExecuteCommandTokenAndDirective.second) {
            ACSDK_ERROR(LX("processExecuteCommandsResultExecutorFailed")
                            .d("reason", "directive to handle is null")
                            .d("messageId", token));
            isSuccess = false;
        }

        if (isSuccess) {
            setHandlingCompleted(m_lastExecuteCommandTokenAndDirective.second);
        } else {
            sendExceptionEncounteredAndReportFailed(
                m_lastExecuteCommandTokenAndDirective.second, "Commands execution failed: " + error);
        }

        m_lastExecuteCommandTokenAndDirective.first.clear();
        executeProactiveStateReport();
    });
}

void AlexaPresentation::processActivityEvent(const std::string& source, const std::string& event) {
    smartScreenSDKInterfaces::ActivityEvent activityEvent = smartScreenSDKInterfaces::activityEventFromString(event);
    if (smartScreenSDKInterfaces::ActivityEvent::UNKNOWN == activityEvent) {
        ACSDK_ERROR(LX("processActivityEventFailed").d("reason", "received unknown type of event"));
        return;
    }

    processActivityEvent(source, activityEvent);
}

void AlexaPresentation::processActivityEvent(
    const std::string& source,
    const smartScreenSDKInterfaces::ActivityEvent activityEvent) {
    if (activityEvent != smartScreenSDKInterfaces::ActivityEvent::ONE_TIME && source.empty()) {
        ACSDK_ERROR(LX("processActivityEventFailed").d("reason", "event source is empty"));
        return;
    }

    m_executor->submit([this, source, activityEvent]() {
        ACSDK_DEBUG5(LX("processActivityEventInExecutor").d("source", source).d("event", (int)activityEvent));
        switch (activityEvent) {
            case smartScreenSDKInterfaces::ActivityEvent::ACTIVATED:
                if (DialogUXState::IDLE == m_dialogUxState && m_activeSources.empty()) {
                    executeStopTimer();
                }
                m_activeSources.insert(source);
                m_documentInteractionState = InteractionState::ACTIVE;
                break;
            case smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED:
                if (m_activeSources.erase(source)) {
                    if (m_activeSources.empty()) {
                        m_documentInteractionState = InteractionState::INACTIVE;
                    }
                    if (DialogUXState::IDLE == m_dialogUxState &&
                        InteractionState::INACTIVE == m_documentInteractionState) {
                        executeStartOrExtendTimer();
                    }
                }
                break;
            case smartScreenSDKInterfaces::ActivityEvent::ONE_TIME:
                if (DialogUXState::IDLE == m_dialogUxState &&
                    InteractionState::INACTIVE == m_documentInteractionState) {
                    executeStartOrExtendTimer();
                }
                break;
            case smartScreenSDKInterfaces::ActivityEvent::INTERRUPT:
                for (auto& observer : m_observers) {
                    observer->interruptCommandSequence(m_lastRenderedAPLToken);
                }
                if (DialogUXState::IDLE == m_dialogUxState &&
                    InteractionState::INACTIVE == m_documentInteractionState) {
                    executeStartOrExtendTimer();
                }
                break;
            default:
                // Not possible as returned above.
                break;
        }
        executeProactiveStateReport();
    });
}

void AlexaPresentation::executeResetActivityTracker() {
    ACSDK_DEBUG5(LX(__func__));
    m_activeSources.clear();
    m_documentInteractionState = InteractionState::INACTIVE;
    executeStopTimer();
}

const std::string AlexaPresentation::getNonAPLDocumentToken() {
    return "NonAPLDocumentToken";
}

void AlexaPresentation::startMetricsEvent(MetricEvent metricEvent) {
    switch (metricEvent) {
        case MetricEvent::RENDER_DOCUMENT:
        case MetricEvent::LAYOUT:
        case MetricEvent::INFLATE:
            m_currentActiveTimePoints[metricEvent] = std::chrono::steady_clock::now();
            break;  // Timer Metric events
        case MetricEvent::TEXT_MEASURE_COUNT:
        case MetricEvent::DROP_FRAME: {
            if (m_currentActiveCountPoints.find(metricEvent) == m_currentActiveCountPoints.end()) {
                m_currentActiveCountPoints[metricEvent] = 0;
            }
            ++m_currentActiveCountPoints[metricEvent];  // always increment by one
            break;                                      // Count Metric events
        }
        default:
            break;
    }
}

void AlexaPresentation::triggerMetricsEventWithData(
    MetricEvent metricEvent,
    uint64_t count,
    const std::string& activityName) {
    switch (metricEvent) {
        case MetricEvent::TEXT_MEASURE_COUNT:
        case MetricEvent::DROP_FRAME: {
            m_currentActiveCountPoints[metricEvent] += count;
            endMetricsEvent(metricEvent, activityName);
            break;
        }
        default:
            ACSDK_DEBUG3(LX(__func__).m("Incorrect event-type for data"));
            break;
    }
}

void AlexaPresentation::triggerMetricsEventWithData(
    MetricEvent metricEvent,
    std::chrono::steady_clock::time_point& tp,
    const std::string& activityName) {
    switch (metricEvent) {
        case MetricEvent::RENDER_DOCUMENT:
        case MetricEvent::LAYOUT:
        case MetricEvent::INFLATE:
            m_currentActiveTimePoints[metricEvent] = tp;
            endMetricsEvent(metricEvent, activityName);
            break;
        default:
            ACSDK_DEBUG3(LX(__func__).m("Incorrect event-type for data"));
            break;
    }
}

void AlexaPresentation::resetMetricsEvent(MetricEvent metricEvent) {
    switch (metricEvent) {
        case MetricEvent::RENDER_DOCUMENT:
        case MetricEvent::LAYOUT:
        case MetricEvent::INFLATE: {
            m_currentActiveTimePoints.erase(metricEvent);
            break;  // Timer Metric events
        }
        case MetricEvent::TEXT_MEASURE_COUNT:
        case MetricEvent::DROP_FRAME: {
            m_currentActiveCountPoints.erase(metricEvent);
            break;  // Count Metric events
        }
        default:
            break;
    }
}

std::string AlexaPresentation::getSkillIdFromAPLToken(const std::string& aplToken) {
    std::string skillId;
    try {
        std::regex rgx(".*#TID#([a-zA-Z0-9-_\\.]+[a-zA-Z0-9]):.*");
        std::smatch match;

        if (std::regex_search(aplToken.begin(), aplToken.end(), match, rgx) &&
            match.size() > 1)  // Magic # we discard the first literal and goto the value
            skillId = match[1].str();
    } catch (const std::regex_error& e) {
        ACSDK_ERROR(LX(__func__).d("reason", e.what()));
    }
    return skillId;
}

void AlexaPresentation::endMetricsEvent(MetricEvent metricEvent, const std::string& activityName) {
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricEvent> event;
    switch (metricEvent) {
        case MetricEvent::RENDER_DOCUMENT:
        case MetricEvent::LAYOUT:
        case MetricEvent::INFLATE: {
            auto endTP = std::chrono::duration_cast<std::chrono::milliseconds>(
                m_currentActiveTimePoints.find(metricEvent) != m_currentActiveTimePoints.end()
                    ? std::chrono::steady_clock::now() - m_currentActiveTimePoints[metricEvent]
                    : std::chrono::milliseconds(0));

            event = alexaClientSDK::avsCommon::utils::metrics::MetricEventBuilder{}
                        .setActivityName(activityName)
                        .setPriority(alexaClientSDK::avsCommon::utils::metrics::Priority::HIGH)
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointDurationBuilder(endTP)
                                          .setName(MetricsDataPointNames[metricEvent])
                                          .build())
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointStringBuilder{}
                                          .setName("APL_TOKEN")
                                          .setValue(m_lastRenderedAPLToken.c_str())
                                          .build())
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointStringBuilder{}
                                          .setName("SKILL_ID")
                                          .setValue(getSkillIdFromAPLToken(m_lastRenderedAPLToken).c_str())
                                          .build())
                        .build();
            m_currentActiveTimePoints.erase(metricEvent);
            break;  // Timer Metric events
        }
        case MetricEvent::TEXT_MEASURE_COUNT:
        case MetricEvent::DROP_FRAME: {
            event = alexaClientSDK::avsCommon::utils::metrics::MetricEventBuilder{}
                        .setActivityName(activityName)
                        .setPriority(alexaClientSDK::avsCommon::utils::metrics::Priority::HIGH)
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointCounterBuilder()
                                          .setName(MetricsDataPointNames[metricEvent])
                                          .increment(m_currentActiveCountPoints[metricEvent])
                                          .build())
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointStringBuilder{}
                                          .setName("APL_TOKEN")
                                          .setValue(m_lastRenderedAPLToken.c_str())
                                          .build())
                        .addDataPoint(alexaClientSDK::avsCommon::utils::metrics::DataPointStringBuilder{}
                                          .setName("SKILL_ID")
                                          .setValue(getSkillIdFromAPLToken(m_lastRenderedAPLToken).c_str())
                                          .build())
                        .build();

            m_currentActiveCountPoints.erase(metricEvent);
            break;  // Count Metric events
        }
        default:
            break;
    }

    if (m_metricRecorder != nullptr) {
        std::lock_guard<std::mutex> lock{m_MetricsRecorderMutex};
        m_metricRecorder->recordMetric(event);
    }  // lock out of scope
}

void AlexaPresentation::recordRenderComplete() {
    ACSDK_DEBUG5(LX(__func__));
    m_documentRendered = true;

    /* The view layout was drawn */
    endMetricsEvent(MetricEvent::LAYOUT, ACTIVITY_VIEW_LAYOUT);

    /* Document was rendered */
    endMetricsEvent(MetricEvent::RENDER_DOCUMENT, ACTIVITY_RENDER_DOCUMENT);
}

void AlexaPresentation::recordDropFrameCount(uint64_t dropFrameCount) {
    triggerMetricsEventWithData(MetricEvent::DROP_FRAME, dropFrameCount, ACTIVITY_DROP_FRAME);
}

void AlexaPresentation::recordAPLEvent(APLClient::AplRenderingEvent event) {
    switch (event) {
        case APLClient::AplRenderingEvent::INFLATE_BEGIN: {
            /* Document will start inflating now */
            startMetricsEvent(MetricEvent::INFLATE);

            break;
        }
        case APLClient::AplRenderingEvent::INFLATE_END: {
            /* APL Core engine completed the context inflate */
            endMetricsEvent(MetricEvent::INFLATE, ACTIVITY_INFLATE_APL);

            /* Text measurement ends after the document is inflated  */
            endMetricsEvent(MetricEvent::TEXT_MEASURE_COUNT, ACTIVITY_TEXT_MEASURE);

            /* Start of the view layout draw*/
            startMetricsEvent(MetricEvent::LAYOUT);

            break;
        }
        case APLClient::AplRenderingEvent::TEXT_MEASURE: {
            /* Text measurement was performed on the document */
            startMetricsEvent(MetricEvent::TEXT_MEASURE_COUNT);
            break;
        }
        default:
            ACSDK_DEBUG3(LX(__func__).m("Unhandled event type"));
    }
}

void AlexaPresentation::executeProactiveStateReport() {
    if (m_stateReportCheckInterval.count() == 0 || !m_lastDisplayedDirective || m_lastRenderedAPLToken.empty() ||
        ALEXA_PRESENTATION_APL_NAMESPACE != m_lastDisplayedDirective->directive->getNamespace() ||
        !m_documentRendered) {
        // Not rendering APL or reporting disabled, do not request a state report
        return;
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> duration = now - m_lastReportTime;

    if (!m_stateReportPending && duration.count() > m_minStateReportInterval.count()) {
        m_stateReportPending = true;
        m_visualStateProvider->provideState(m_lastRenderedAPLToken, PROACTIVE_STATE_REQUEST_TOKEN);
    }
}

void AlexaPresentation::proactiveStateReport() {
    m_executor->submit([this] { executeProactiveStateReport(); });
}

void AlexaPresentation::notifyAbort() {
    for (auto observer : m_observers) {
        observer->onRenderingAborted(m_lastRenderedAPLToken);
    }
}

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
