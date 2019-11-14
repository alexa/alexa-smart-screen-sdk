/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/AVS/EventBuilder.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Logger/Logger.h>

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

/// AlexaPresentation capability constants
/// AlexaPresentation interface type
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";

/// AlexaPresentation interface name2
static const std::string ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_NAME = "Alexa.Presentation.APL";

/// AlexaPresentation interface version for Alexa.Presentation.APL
static const std::string ALEXAPRESENTATIONAPL_CAPABILITY_INTERFACE_VERSION = "1.0";

/// AlexaPresentation interface name3
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_NAME = "Alexa.Presentation";

/// AlexaPresentation interface version for Alexa.Presentation.
static const std::string ALEXAPRESENTATION_CAPABILITY_INTERFACE_VERSION = "1.0";

/// String to identify log entries originating from this file.
static const std::string TAG{"AlexaPresentation"};

/// The key in our config file to find the root of APL Presentation configuration.
static const std::string ALEXAPRESENTATION_CONFIGURATION_ROOT_KEY = "alexaPresentationCapabilityAgent";

/// The key in our config file to set the display card timeout value for RenderDocument case
static const std::string ALEXAPRESENTATION_DOCUMENT_INTERACTION_KEY = "displayDocumentInteractionIdleTimeout";

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

/// The name for UserEvent event.
static const std::string USER_EVENT{"UserEvent"};

/// The name for DocumentDismissed event.
static const std::string DOCUMENT_DISMISSED{"Dismissed"};

/// The RenderDocument directive signature.
static const NamespaceAndName DOCUMENT{ALEXA_PRESENTATION_APL_NAMESPACE, RENDER_DOCUMENT};

/// The ExecuteCommand directive signature.
static const NamespaceAndName COMMAND{ALEXA_PRESENTATION_APL_NAMESPACE, EXECUTE_COMMAND};

/// Name of the runtime configuration.
static const std::string RUNTIME_CONFIG = "runtime";

/// Identifier for the runtime (APL) version of the configuration.
static const std::string APL_MAX_VERSION = "maxVersion";

/// Identifier for the presentationToken's sent in a RenderDocument directive
static const std::string PRESENTATION_TOKEN = "presentationToken";

/// Identifier for the windowId's sent in a RenderDocument directive
static const std::string WINDOW_ID = "windowId";

/// Identifier for the document sent in a RenderDocument directive
static const std::string DOCUMENT_FIELD = "document";

/// Identifier for the commands sent in a RenderDocument directive
static const std::string COMMANDS_FIELD = "commands";

/// Tag for finding the visual context information sent from the runtime as part of event context.
static const std::string VISUAL_CONTEXT_NAME{"RenderedDocumentState"};

/// Default timeout for clearing the RenderDocument display card when there is no interaction happening.
static const std::chrono::milliseconds DEFAULT_DOCUMENT_INTERACTION_TIMEOUT_MS{30000};

/// The AlexaPresentation context state signature.
static const avsCommon::avs::NamespaceAndName RENDERED_DOCUMENT_STATE{ALEXA_PRESENTATION_APL_NAMESPACE,
                                                                      VISUAL_CONTEXT_NAME};

std::shared_ptr<AlexaPresentation> AlexaPresentation::create(
    std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider) {
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

    std::shared_ptr<AlexaPresentation> alexaPresentation(
        new AlexaPresentation(focusManager, exceptionSender, messageSender, contextManager, visualStateProvider));

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

    // If key is present, then read and initialize the value from config or set to default.
    configurationRoot.getDuration<std::chrono::milliseconds>(
        ALEXAPRESENTATION_DOCUMENT_INTERACTION_KEY,
        &m_sdkConfiguredDocumentInteractionTimeout,
        DEFAULT_DOCUMENT_INTERACTION_TIMEOUT_MS);

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
    ACSDK_DEBUG5(LX(__func__));
    if (!info || !info->directive) {
        ACSDK_ERROR(LX("handleDirectiveFailed").d("reason", "nullDirectiveInfo"));
        return;
    }

    if (info->directive->getNamespace() == DOCUMENT.nameSpace && info->directive->getName() == DOCUMENT.name) {
        setDocumentIdleTimeout(m_sdkConfiguredDocumentInteractionTimeout);
        handleRenderDocumentDirective(info);
    } else if (info->directive->getNamespace() == COMMAND.nameSpace && info->directive->getName() == COMMAND.name) {
        handleExecuteCommandDirective(info);
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
    return configuration;
}

void AlexaPresentation::clearCard() {
    ACSDK_DEBUG5(LX(__func__));
    m_executor->submit([this]() {
        ACSDK_DEBUG5(LX("clearCardExecutor"));
        executeResetActivityTracker();
        executeClearCardEvent();
        // TODO: ARC-575 consider cleaning the commands on CommandsSequencer
        doClearExecuteCommand("Card cleared");
    });
}

void AlexaPresentation::clearAllExecuteCommands() {
    m_executor->submit([this]() { doClearExecuteCommand("User exited"); });
}

void AlexaPresentation::doClearExecuteCommand(const std::string& reason) {
    ACSDK_DEBUG5(LX(__func__));
    if (!(m_lastExecuteCommandTokenAndDirective.first.empty()) && m_lastExecuteCommandTokenAndDirective.second &&
        m_lastExecuteCommandTokenAndDirective.second->result) {
        m_lastExecuteCommandTokenAndDirective.second->result->setFailed(reason);
    }

    m_lastExecuteCommandTokenAndDirective.first.clear();
}

void AlexaPresentation::onFocusChanged(avsCommon::avs::FocusState newFocus) {
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
                executeStartTimer(m_documentInteractionTimeout);
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
        if (!m_observers.insert(observer).second) {
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
    std::shared_ptr<avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> visualStateProvider) :
        CapabilityAgent{ALEXA_PRESENTATION_NAMESPACE, exceptionSender},
        RequiresShutdown{"AlexaPresentation"},
        m_focus{FocusState::NONE},
        m_state{smartScreenSDKInterfaces::State::IDLE},
        m_dialogUxState{DialogUXState::IDLE},
        m_focusManager{focusManager},
        m_messageSender{messageSender},
        m_contextManager{contextManager},
        m_visualStateProvider{visualStateProvider},
        m_APLVersion{},
        m_documentInteractionState{AlexaPresentation::InteractionState::INACTIVE} {
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
    m_executor->submit([this, payload]() {
        m_events.push(std::make_pair(AlexaPresentationEvents::APL_USER_EVENT, payload));
        m_contextManager->getContext(shared_from_this());
    });
}

void AlexaPresentation::doShutdown() {
    m_executor->shutdown();

    doClearExecuteCommand("RenderingHandlerShuttingDown");

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
    if (info && info->result) {
        info->result->setCompleted();
    }
    removeDirective(info);
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
            return;
        }

        std::string presentationToken;
        if (!jsonUtils::retrieveValue(payload, PRESENTATION_TOKEN, &presentationToken)) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor").d("reason", "NoPresentationToken"));
            sendExceptionEncounteredAndReportFailed(info, "missing presentationToken");
            return;
        }

        std::string APLdocument;
        if (!jsonUtils::retrieveValue(payload, DOCUMENT_FIELD, &APLdocument)) {
            ACSDK_ERROR(LX("handleRenderDocumentDirectiveFailedInExecutor").d("reason", "NoDocument"));
            sendExceptionEncounteredAndReportFailed(info, "missing APLdocument");
            return;
        }

        executeDisplayCardEvent(info);
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

        m_lastExecuteCommandTokenAndDirective = std::make_pair(info->directive->getMessageId(), info);
        executeExecuteCommandEvent(info);
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

    for (auto& observer : m_observers) {
        if (isClearCard) {
            observer->clearDocument();
        } else {
            observer->renderDocument(m_lastDisplayedDirective->directive->getPayload(), newToken, windowId);
        }
    }

    if (dismissPrevious) {
        /*
         * Send @c Dismissed event for the previous document
         * Whether we are displaying new card or just dismissing this one.
         */
        executeSendDismissedEvent();
    }

    m_lastRenderedAPLToken = newToken;
}

void AlexaPresentation::executeDisplayCard() {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective &&
        m_lastDisplayedDirective->directive->getNamespace() == ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT) {
        executeResetActivityTracker();
        executeRenderDocumentCallbacks(false);
    }
}

void AlexaPresentation::executeCommandEvent(std::shared_ptr<DirectiveInfo> info) {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective->directive->getNamespace() != ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() != RENDER_DOCUMENT) {
        sendExceptionEncounteredAndReportFailed(info, "APL document that requires command executed is not rendered.");
        ACSDK_ERROR(LX("executeCommandEventFailed")
                        .d("reason", "Cannot execute command when an APL document is not rendered."));
        return;
    }

    for (auto& observer : m_observers) {
        observer->executeCommands(info->directive->getPayload(), info->directive->getMessageId());
    }
}

void AlexaPresentation::executeClearCard() {
    ACSDK_DEBUG5(LX(__func__));

    if (m_lastDisplayedDirective &&
        m_lastDisplayedDirective->directive->getNamespace() == ALEXA_PRESENTATION_APL_NAMESPACE &&
        m_lastDisplayedDirective->directive->getName() == RENDER_DOCUMENT) {
        executeRenderDocumentCallbacks(true);
    }
}

void AlexaPresentation::executeRestartTimer(std::chrono::milliseconds timeout) {
    if (smartScreenSDKInterfaces::State::DISPLAYING == m_state) {
        ACSDK_DEBUG3(LX(__func__).d("timeoutInMilliseconds", timeout.count()));
        m_idleTimer.stop();
        m_idleTimer.start(timeout, [this] { m_executor->submit([this] { executeClearCardEvent(); }); });
    }
}

void AlexaPresentation::executeStartTimer(std::chrono::milliseconds timeout) {
    if (smartScreenSDKInterfaces::State::DISPLAYING == m_state) {
        ACSDK_DEBUG3(LX(__func__).d("timeoutInMilliseconds", timeout.count()));
        m_idleTimer.start(timeout, [this] { m_executor->submit([this] { executeClearCardEvent(); }); });
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
 * displayCard - This event happens when the AlexaPresentation is ready to notify its observers to display a
 * displayCard.
 *
 * focusChanged - This event happens when the @c FocusManager notifies a change in @c FocusState in the visual
 * channel.
 *
 * timer - This event happens when m_idleTimer expires and needs to notify its observers to clear the
 * displayCard.
 *
 * cardCleared - This event happens when @c displayCardCleared() is called to notify @c RenderingHandler the device has
 * cleared the screen.
 *
 * Each state transition may result in one or more of the following actions:
 * (A) Acquire channel
 * (B) Release channel
 * (C) Notify observers to display displayCard
 * (D) Notify observers to clear displayCard
 * (E) Log error about unexpected focusChanged event.
 *
 * Below is the state table illustrating the state transition and its action.  NC means no change in state.
 *
 *                                              E  V  E  N  T  S
 *                -----------------------------------------------------------------------------------------
 *  Current State | displayCard  | timer          | focusChanged::NONE | focusChanged::FG/BG | cardCleared
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
                    executeDisplayCard();
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
                    executeDisplayCard();
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

void AlexaPresentation::executeDisplayCardEvent(
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
                executeDisplayCard();
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
            executeCommandEvent(info);
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

std::unordered_set<std::shared_ptr<avsCommon::avs::CapabilityConfiguration>> AlexaPresentation::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void AlexaPresentation::executeSendDismissedEvent() {
    ACSDK_DEBUG5(LX(__func__).d("Token", m_lastRenderedAPLToken));

    if (m_lastRenderedAPLToken.empty()) {
        return;
    }

    // Assemble the event payload.
    std::ostringstream payload;
    payload << R"({"presentationToken":")" << m_lastRenderedAPLToken << R"("})";

    m_events.push(std::make_pair(AlexaPresentationEvents::APL_DISMISSED, payload.str()));
    ACSDK_DEBUG5(LX(__func__).d("APL_TOKEN", m_lastRenderedAPLToken).m("Pushed APL Dismissed"));
    m_contextManager->getContext(shared_from_this());
}

void AlexaPresentation::onContextAvailable(const std::string& jsonContext) {
    auto task = [this, jsonContext]() {
        ACSDK_DEBUG9(LX("onContextAvailableExecutor"));

        if (!m_events.empty()) {
            auto event = m_events.front();

            switch (event.first) {
                case AlexaPresentationEvents::APL_USER_EVENT: {
                    auto msgIdAndJsonEvent = avsCommon::avs::buildJsonEventString(
                        ALEXA_PRESENTATION_APL_NAMESPACE, USER_EVENT, "", event.second, jsonContext);
                    auto userEventMessage = std::make_shared<avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);

                    ACSDK_DEBUG9(LX("onContextAvailable : Send UserEvent to AVS."));
                    m_messageSender->sendMessage(userEventMessage);
                    break;
                }
                case AlexaPresentationEvents::APL_DISMISSED: {
                    auto msgIdAndJsonEvent = avsCommon::avs::buildJsonEventString(
                        ALEXA_PRESENTATION_NAMESPACE, DOCUMENT_DISMISSED, "", event.second, jsonContext);
                    auto documentDismissedEvent =
                        std::make_shared<avsCommon::avs::MessageRequest>(msgIdAndJsonEvent.second);

                    ACSDK_DEBUG9(LX("onContextAvailable : Send documentDismissed event to AVS."));
                    m_messageSender->sendMessage(documentDismissedEvent);
                    break;
                }
            }

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
        m_visualStateProvider->provideState(stateRequestToken);
    } else {
        m_contextManager->setState(RENDERED_DOCUMENT_STATE, "", StateRefreshPolicy::SOMETIMES, stateRequestToken);
    }
}

void AlexaPresentation::onVisualContextAvailable(const unsigned int requestToken, const std::string& payload) {
    ACSDK_DEBUG3(LX(__func__).d("requestToken", requestToken));
    m_executor->submit([this, requestToken, payload]() {
        ACSDK_DEBUG3(LX("onVisualContextAvailableExecutor"));
        m_contextManager->setState(RENDERED_DOCUMENT_STATE, payload, StateRefreshPolicy::ALWAYS, requestToken);
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
    m_documentInteractionTimeout = timeout;
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

        if (m_lastRenderedAPLToken != token) {
            ACSDK_ERROR(LX("processRenderDocumentResultFailedInExecutor")
                            .d("reason", "tokenMismatch")
                            .d("expected", m_lastRenderedAPLToken)
                            .d("actual", token));
            return;
        }

        if (result) {
            setHandlingCompleted(m_lastDisplayedDirective);
        } else {
            sendExceptionEncounteredAndReportFailed(m_lastDisplayedDirective, "Renderer failed: " + error);
        }

        if (DialogUXState::IDLE == m_dialogUxState && InteractionState::INACTIVE == m_documentInteractionState) {
            executeStartTimer(m_sdkConfiguredDocumentInteractionTimeout);
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
                        executeStartTimer(m_sdkConfiguredDocumentInteractionTimeout);
                    }
                }
                break;
            case smartScreenSDKInterfaces::ActivityEvent::ONE_TIME:
                if (DialogUXState::IDLE == m_dialogUxState &&
                    InteractionState::INACTIVE == m_documentInteractionState) {
                    executeRestartTimer(m_sdkConfiguredDocumentInteractionTimeout);
                }
                break;
            case smartScreenSDKInterfaces::ActivityEvent::INTERRUPT:
                for (auto& observer : m_observers) {
                    observer->interruptCommandSequence();
                }
                if (DialogUXState::IDLE == m_dialogUxState &&
                    InteractionState::INACTIVE == m_documentInteractionState) {
                    executeRestartTimer(m_sdkConfiguredDocumentInteractionTimeout);
                }
                break;
            default:
                // Not possible as returned above.
                break;
        }
    });
}

void AlexaPresentation::executeResetActivityTracker() {
    ACSDK_DEBUG5(LX(__func__));
    m_activeSources.clear();
    m_documentInteractionState = InteractionState::INACTIVE;
    executeStopTimer();
}

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
