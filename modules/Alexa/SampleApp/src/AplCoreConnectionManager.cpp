/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Timing/Timer.h>
#include "SampleApp/AplCoreTextMeasurement.h"
#include "SampleApp/AplCoreConnectionManager.h"
#include "SampleApp/Messages/AplCoreViewhostMessage.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::timing;

using namespace smartScreenSDKInterfaces;

static const std::string TAG{"AplCoreConnectionManager"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)
/// The keys used in executeProvideState.
static const char TOKEN_KEY[] = "token";
static const char VERSION_KEY[] = "version";
static const char CONTEXT_KEY[] = "componentsVisibleOnScreen";
/// The value used in executeProvideState.
// TODO: need to get version number from APLCoreEngine: ARC-858
static const char VERSION_VALUE[] = "AplRenderer-1.2";

/// APL Scaling bias constant
static const float SCALING_BIAS_CONSTANT = 10.0f;
/// APL Scaling cost override
static const bool SCALING_SHAPE_OVERRIDES_COST = true;

/// The keys used in APL context creation.
static const char HEIGHT_KEY[] = "height";
static const char WIDTH_KEY[] = "width";
static const char DPI_KEY[] = "dpi";
static const char MODE_KEY[] = "mode";
static const char SHAPE_KEY[] = "shape";
static const char SCALING_KEY[] = "scaling";
static const char SCALE_FACTOR_KEY[] = "scaleFactor";
static const char VIEWPORT_WIDTH_KEY[] = "viewportWidth";
static const char VIEWPORT_HEIGHT_KEY[] = "viewportHeight";
static const char HIERARCHY_KEY[] = "hierarchy";
static const char X_KEY[] = "x";
static const char Y_KEY[] = "y";
static const char DOCTHEME_KEY[] = "docTheme";
static const char ENSURELAYOUT_KEY[] = "ensureLayout";
static const char AGENTNAME_KEY[] = "agentName";
static const char AGENTVERSION_KEY[] = "agentVersion";
static const char ALLOWOPENURL_KEY[] = "allowOpenUrl";
static const char DISALLOWVIDEO_KEY[] = "disallowVideo";
static const char ANIMATIONQUALITY_KEY[] = "animationQuality";

// The keys used in APL event execution.
static const char ERROR_KEY[] = "error";
static const char EVENT_KEY[] = "event";
static const char EVENT_TERMINATE_KEY[] = "eventTerminate";
static const char DIRTY_KEY[] = "dirty";

/// SendEvent keys
static const char PRESENTATION_TOKEN_KEY[] = "presentationToken";
static const char SOURCE_KEY[] = "source";
static const char ARGUMENTS_KEY[] = "arguments";
static const char COMPONENTS_KEY[] = "components";

/// Media update keys
static const char MEDIA_STATE_KEY[] = "mediaState";
static const char FROM_EVENT_KEY[] = "fromEvent";
static const char TRACK_INDEX_KEY[] = "trackIndex";
static const char TRACK_COUNT_KEY[] = "trackCount";
static const char CURRENT_TIME_KEY[] = "currentTime";
static const char DURATION_KEY[] = "duration";
static const char PAUSED_KEY[] = "paused";
static const char ENDED_KEY[] = "ended";

/// Activity tracking sources
static const std::string APL_COMMAND_EXECUTION{"APLCommandExecution"};
static const std::string APL_SCREEN_LOCK{"APLScreenLock"};
static const char RENDERING_OPTIONS_KEY[] = "renderingOptions";

static const char LEGACY_KARAOKE_KEY[] = "legacyKaraoke";

static apl::Bimap<std::string, apl::ViewportMode> AVS_VIEWPORT_MODE_MAP = {
    {"HUB", apl::ViewportMode::kViewportModeHub},
    {"TV", apl::ViewportMode::kViewportModeTV},
    {"MOBILE", apl::ViewportMode::kViewportModeMobile},
    {"AUTO", apl::ViewportMode::kViewportModeAuto},
    {"PC", apl::ViewportMode::kViewportModePC},
};

static apl::Bimap<std::string, apl::ScreenShape> AVS_VIEWPORT_SHAPE_MAP = {
    {"ROUND", apl::ScreenShape::ROUND},
    {"RECTANGLE", apl::ScreenShape::RECTANGLE},
};

AplCoreConnectionManager::AplCoreConnectionManager(const std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClientInterface) :
        m_guiClientInterface{guiClientInterface},
        m_ScreenLock{false},
        m_SequenceNumber{0},
        m_replyExpectedSequenceNumber{0},
        m_blockingSendReplyExpected{false} {
    m_StartTime = getCurrentTime();
    m_messageHandlers.emplace("build", [this](const rapidjson::Value& payload) { handleBuild(payload); });
    m_messageHandlers.emplace("update", [this](const rapidjson::Value& payload) { handleUpdate(payload); });
    m_messageHandlers.emplace("updateMedia", [this](const rapidjson::Value& payload) { handleMediaUpdate(payload); });
    m_messageHandlers.emplace(
        "updateGraphic", [this](const rapidjson::Value& payload) { handleGraphicUpdate(payload); });
    m_messageHandlers.emplace("response", [this](const rapidjson::Value& payload) { handleEventResponse(payload); });
    m_messageHandlers.emplace("ensureLayout", [this](const rapidjson::Value& payload) { handleEnsureLayout(payload); });
    m_messageHandlers.emplace(
        "scrollToRectInComponent", [this](const rapidjson::Value& payload) { handleScrollToRectInComponent(payload); });
    m_messageHandlers.emplace(
        "handleKeyboard", [this](const rapidjson::Value& payload) { handleHandleKeyboard(payload); });
    m_messageHandlers.emplace(
        "updateCursorPosition", [this](const rapidjson::Value& payload) { handleUpdateCursorPosition(payload); });
}

void AplCoreConnectionManager::setContent(
    const apl::ContentPtr content,
    const std::string& token,
    const std::string& windowId) {
    m_executor.submit([this, content, token, windowId] {
        m_Content = content;
        m_aplToken = token;
        auto message = messages::AplRenderMessage(windowId, token);
        m_guiClientInterface->sendMessage(message);
    });
}

void AplCoreConnectionManager::setSupportedViewports(const std::string& jsonPayload) {
    m_executor.submit([this, jsonPayload] {
        rapidjson::Document* document = new rapidjson::Document();
        if (!jsonUtils::parseJSON(jsonPayload, document) || document->GetType() != rapidjson::Type::kArrayType) {
            ACSDK_ERROR(LX("setSupportedViewportsFailed").d("reason", "Directive payload parse failed"));
            return;
        }

        m_ViewportSizeSpecifications.clear();
        for (auto& spec : document->GetArray()) {
            double minWidth = getOptionalValue(spec, "minWidth", 1);
            double maxWidth = getOptionalValue(spec, "maxWidth", INT_MAX);
            double minHeight = getOptionalValue(spec, "minHeight", 1);
            double maxHeight = getOptionalValue(spec, "maxHeight", INT_MAX);
            std::string mode = getOptionalValue(spec, "mode", "HUB");
            std::string shape = spec.FindMember("shape")->value.GetString();

            m_ViewportSizeSpecifications.emplace_back(
                minWidth,
                maxWidth,
                minHeight,
                maxHeight,
                AVS_VIEWPORT_MODE_MAP.at(mode),
                AVS_VIEWPORT_SHAPE_MAP.at(shape) == apl::ScreenShape::ROUND);
        }
    });
}

double AplCoreConnectionManager::getOptionalValue(
    const rapidjson::Value& jsonNode,
    const std::string& key,
    double defaultValue) {
    double value = defaultValue;
    const auto& valueIt = jsonNode.FindMember(key);
    if (valueIt != jsonNode.MemberEnd()) {
        value = valueIt->value.GetDouble();
    }

    return value;
}

std::string AplCoreConnectionManager::getOptionalValue(
    const rapidjson::Value& jsonNode,
    const std::string& key,
    const std::string& defaultValue) {
    std::string value = defaultValue;
    const auto& valueIt = jsonNode.FindMember(key);
    if (valueIt != jsonNode.MemberEnd()) {
        value = valueIt->value.GetString();
    }

    return value;
}

bool AplCoreConnectionManager::getOptionalBool(
    const rapidjson::Value& jsonNode,
    const std::string& key,
    bool defaultValue) {
    bool value = defaultValue;
    const auto& valueIt = jsonNode.FindMember(key);
    if (valueIt != jsonNode.MemberEnd()) {
        value = valueIt->value.GetBool();
    }

    return value;
}

int AplCoreConnectionManager::getOptionalInt(
    const rapidjson::Value& jsonNode,
    const std::string& key,
    int defaultValue) {
    if (jsonNode.HasMember(key) && jsonNode[key].IsInt()) {
        return jsonNode[key].GetInt();
    }

    return defaultValue;
}

void AplCoreConnectionManager::onMessage(const std::string& message) {
    // Check if this matches a pending request before scheduling it with the executor as blockingSend may be running
    // on the m_executor thread
    uint64_t seqno;
    jsonUtils::retrieveValue(message, "seqno", &seqno);
    if (m_blockingSendReplyExpected && seqno == m_replyExpectedSequenceNumber) {
        m_blockingSendReplyExpected = false;
        m_replyPromise.set_value(message);
        return;
    }

    m_executor.submit([this, message] {
        rapidjson::Document doc;
        if (!jsonUtils::parseJSON(message, &doc)) {
            ACSDK_ERROR(LX("onMessageFailedInExecutor").d("reason", "parsingFailed"));
            return;
        }

        std::string type;
        if (!jsonUtils::retrieveValue(doc, "type", &type)) {
            ACSDK_ERROR(LX("onMessageFailedInExecutor").d("reason", "Unable to find type in message"));
            return;
        }

        auto payload = doc.FindMember("payload");
        if (payload == doc.MemberEnd()) {
            ACSDK_ERROR(
                LX("onMessageFailedInExecutor").d("reason", "Unable to find payload in message type").d("type", type));
            return;
        }

        auto fit = m_messageHandlers.find(type);
        if (fit != m_messageHandlers.end()) {
            fit->second(payload->value);
        } else {
            ACSDK_ERROR(LX("onMessageFailedInExecutor").d("reason", "Unrecognized message type").d("type", type));
        }
    });
}

void AplCoreConnectionManager::executeCommands(const std::string& command, const std::string& token) {
    m_executor.submit([this, command, token] {
        ACSDK_DEBUG5(LX("executeCommandsInExecutor").d("token", token));
        if (!m_Root) {
            ACSDK_ERROR(LX("executeCommandsFailedInExecutor").d("reason", "Root context is missing"));
            return;
        }

        rapidjson::Document* document = new rapidjson::Document();
        if (!jsonUtils::parseJSON(command, document)) {
            ACSDK_ERROR(LX("executeCommandsFailedInExecutor").d("reason", "Parse commands failed"));
            return;
        }

        auto it = document->FindMember("commands");
        if (it == document->MemberEnd() || it->value.GetType() != rapidjson::Type::kArrayType) {
            ACSDK_ERROR(LX("executeCommandsFailedInExecutor").d("reason", "Missing commands, or is not array"));
            return;
        }

        apl::Object object{it->value};
        auto action = m_Root->executeCommands(object, false);
        if (!action) {
            ACSDK_ERROR(LX("executeCommandsFailedInExecutor").d("reason", "Execute commands failed"));
            return;
        }

        m_guiManager->handleActivityEvent(APL_COMMAND_EXECUTION, smartScreenSDKInterfaces::ActivityEvent::ACTIVATED);

        action->setUserData(document);

        action->then([this, document, token](const apl::ActionPtr& action) {
            ACSDK_DEBUG0(LX("executeCommands").m("Command sequence complete"));
            if (action->getUserData()) {
                delete document;
                action->setUserData(nullptr);
            }
            m_guiManager->handleExecuteCommandsResult(token, true, "");

            m_guiManager->handleActivityEvent(
                APL_COMMAND_EXECUTION, smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
        });
        action->addTerminateCallback([this, action, document, token](const std::shared_ptr<apl::Timers>&) {
            ACSDK_DEBUG0(LX("executeCommandsFailed").m("Command sequence failed"));
            if (action->getUserData()) {
                delete document;
                action->setUserData(nullptr);
            }
            m_guiManager->handleExecuteCommandsResult(token, false, "");

            m_guiManager->handleActivityEvent(
                APL_COMMAND_EXECUTION, smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
        });
    });
}

void AplCoreConnectionManager::provideState(const unsigned int stateRequestToken) {
    if (!m_Content) {
        ACSDK_WARN(LX(__func__).d("reason", "Root context is null"));
        sendError("Root context is null");
        return;
    }

    ACSDK_DEBUG3(LX(__func__).d("stateRequestToken", stateRequestToken));
    m_executor.submit([this, stateRequestToken] { executeProvideState(stateRequestToken); });
}

void AplCoreConnectionManager::executeProvideState(unsigned int stateRequestToken) {
    rapidjson::Document state(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = state.GetAllocator();
    // Add presentation token info
    state.AddMember(TOKEN_KEY, m_aplToken, allocator);
    // Add version info
    state.AddMember(VERSION_KEY, VERSION_VALUE, allocator);
    rapidjson::Value arr(rapidjson::kArrayType);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (m_Root && m_Root->topComponent()) {
        auto context = m_Root->topComponent()->serializeVisualContext(allocator);
        arr.PushBack(context, allocator);
    } else {
        ACSDK_ERROR(LX(__func__).d("reason", "Unable to get visual context"));
        rapidjson::Value emptyObj(rapidjson::kObjectType);
        // add an empty visual context
        arr.PushBack(emptyObj, allocator);
    }
    // Add visual context info
    state.AddMember(CONTEXT_KEY, arr, allocator);
    state.Accept(writer);
    ACSDK_DEBUG3(LX(__func__).d("VisualContext", buffer.GetString()));
    m_guiManager->handleVisualContext(stateRequestToken, buffer.GetString());
}

void AplCoreConnectionManager::interruptCommandSequence() {
    m_executor.submit([this] {
        ACSDK_DEBUG5(LX("interruptCommandSequenceInExecutor"));
        if (m_Root) {
            m_Root->cancelExecution();
        }
    });
}

void AplCoreConnectionManager::handleBuild(const rapidjson::Value& message) {
    auto renderingOptionsMsg = messages::AplCoreViewhostMessage(RENDERING_OPTIONS_KEY);
    rapidjson::Value renderingOptions(rapidjson::kObjectType);
    renderingOptions.AddMember(LEGACY_KARAOKE_KEY, m_Content->getAPLVersion() == "1.0", renderingOptionsMsg.alloc());
    send(renderingOptionsMsg.setPayload(std::move(renderingOptions)));

    if (!m_Content) {
        ACSDK_WARN(LX("handleBuildFailed").d("reason", "No content to build"));
        sendError("No content to build");
        return;
    }

    std::string agentName = getOptionalValue(message, AGENTNAME_KEY, "wssHost");
    std::string agentVersion = getOptionalValue(message, AGENTVERSION_KEY, "1.0");
    bool allowOpenUrl = getOptionalBool(message, ALLOWOPENURL_KEY, false);
    bool disallowVideo = getOptionalBool(message, DISALLOWVIDEO_KEY, false);
    int animationQuality =
        getOptionalInt(message, ANIMATIONQUALITY_KEY, apl::RootConfig::AnimationQuality::kAnimationQualityNormal);

    // TODO: Imports on CDN got wrong APL spec versions. Should be fixed for everyone.
    auto config = apl::RootConfig()
                      .agent(agentName, agentVersion)
                      .allowOpenUrl(allowOpenUrl)
                      .disallowVideo(disallowVideo)
                      .animationQuality(static_cast<apl::RootConfig::AnimationQuality>(animationQuality))
                      .measure(std::make_shared<AplCoreTextMeasurement>(shared_from_this()))
                      .enforceAPLVersion(apl::APLVersion::kAPLVersionIgnore);

    m_PendingEvents.clear();

    // Release the activity tracker
    m_guiManager->handleActivityEvent(APL_COMMAND_EXECUTION, smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);

    if (m_ScreenLock) {
        m_guiManager->handleActivityEvent(APL_SCREEN_LOCK, smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
        m_ScreenLock = false;
    }

    // Handle metrics data
    m_Metrics.size(message[WIDTH_KEY].GetInt(), message[HEIGHT_KEY].GetInt())
        .dpi(message[DPI_KEY].GetInt())
        .shape(AVS_VIEWPORT_SHAPE_MAP.at(message[SHAPE_KEY].GetString()))
        .mode(AVS_VIEWPORT_MODE_MAP.at(message[MODE_KEY].GetString()));

    do {
        apl::ScalingOptions scalingOptions = {
            m_ViewportSizeSpecifications, SCALING_BIAS_CONSTANT, SCALING_SHAPE_OVERRIDES_COST};
        if (!scalingOptions.getSpecifications().empty()) {
            m_AplCoreMetrics = new AplCoreMetrics(m_Metrics, scalingOptions);
        } else {
            m_AplCoreMetrics = new AplCoreMetrics(m_Metrics);
        }

        // Send scaling metrics out to viewhost
        auto reply = messages::AplCoreViewhostMessage(SCALING_KEY);
        rapidjson::Value scaling(rapidjson::kObjectType);
        scaling.AddMember(SCALE_FACTOR_KEY, m_AplCoreMetrics->toViewhost(1.0f), reply.alloc());
        scaling.AddMember(VIEWPORT_WIDTH_KEY, m_AplCoreMetrics->getViewhostWidth(), reply.alloc());
        scaling.AddMember(VIEWPORT_HEIGHT_KEY, m_AplCoreMetrics->getViewhostHeight(), reply.alloc());
        send(reply.setPayload(std::move(scaling)));

        m_StartTime = getCurrentTime();
        m_Root = apl::RootContext::create(m_AplCoreMetrics->getMetrics(), m_Content, config);
        if (m_Root) {
            break;
        } else if (!m_ViewportSizeSpecifications.empty()) {
            ACSDK_WARN(LX("createAplContextFail")
                           .d("reason", "Unable to inflate document with current chosen scaling.")
                           .d("spec", m_AplCoreMetrics->getChosenSpec().toDebugString()));
        }

        auto it = m_ViewportSizeSpecifications.begin();
        for (; it != m_ViewportSizeSpecifications.end(); it++) {
            if (*it == m_AplCoreMetrics->getChosenSpec()) {
                m_ViewportSizeSpecifications.erase(it);
                break;
            }
        }
        if (it == m_ViewportSizeSpecifications.end()) {
            // Core returned specification that is not in list. Something went wrong. Prevent infinite loop.
            break;
        }
    } while (!m_ViewportSizeSpecifications.empty());

    if (m_Root) {
        sendDocumentThemeMessage();

        auto reply = messages::AplCoreViewhostMessage(HIERARCHY_KEY);
        send(reply.setPayload(m_Root->topComponent()->serialize(reply.alloc())));

        auto idleTimeout = std::chrono::milliseconds(m_Root->settings().idleTimeout());
        m_guiManager->setDocumentIdleTimeout(idleTimeout);
        m_guiManager->handleRenderDocumentResult(m_aplToken, true, "");
    } else {
        ACSDK_ERROR(LX("handleBuildFailed").d("reason", "Unable to inflate document"));
        sendError("Unable to inflate document");
        m_guiManager->handleRenderDocumentResult(m_aplToken, false, "Unable to inflate document");
    }
}

void AplCoreConnectionManager::sendDocumentThemeMessage() {
    if (m_Root) {
        auto themeMsg = messages::AplCoreViewhostMessage(DOCTHEME_KEY);
        auto& alloc = themeMsg.alloc();
        rapidjson::Value payload(rapidjson::kObjectType);
        std::string docTheme = "dark";
        if (m_Root->contextPtr()) {
            docTheme = m_Root->contextPtr()->getTheme();
        }
        payload.AddMember(DOCTHEME_KEY, rapidjson::Value(docTheme.c_str(), alloc).Move(), alloc);
        themeMsg.setPayload(std::move(payload));
        send(themeMsg);
    }
}

void AplCoreConnectionManager::handleUpdate(const rapidjson::Value& update) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleUpdateFailed").d("reason", "Root context is null"));
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        ACSDK_ERROR(LX("handleUpdateFailed").d("reason", "Unable to find component").d("id", id));
        sendError("Unable to find component");
        return;
    }

    auto type = static_cast<apl::UpdateType>(update["type"].GetInt());
    auto value = update["value"].GetFloat();
    if (type == apl::UpdateType::kUpdateScrollPosition) {
        value = m_AplCoreMetrics->toCore(value);
    }

    component->update(type, value);
    runEventLoop();
}

void AplCoreConnectionManager::handleMediaUpdate(const rapidjson::Value& update) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleMediaUpdateFailed").d("reason", "Root context is null"));
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        ACSDK_ERROR(LX("handleMediaUpdateFailed").d("reason", "Unable to find component").d("id", id));
        sendError("Unable to find component");
        return;
    }

    if (!update.HasMember(MEDIA_STATE_KEY) || !update.HasMember(FROM_EVENT_KEY)) {
        ACSDK_ERROR(LX("handleMediaUpdateFailed").d("reason", "State update object is missing parameters"));
        sendError("Can't update media state.");
        return;
    }
    auto& state = update[MEDIA_STATE_KEY];
    auto fromEvent = update[FROM_EVENT_KEY].GetBool();

    if (!state.HasMember(TRACK_INDEX_KEY) || !state.HasMember(TRACK_COUNT_KEY) || !state.HasMember(CURRENT_TIME_KEY) ||
        !state.HasMember(DURATION_KEY) || !state.HasMember(PAUSED_KEY) || !state.HasMember(ENDED_KEY)) {
        ACSDK_ERROR(
            LX("handleMediaUpdateFailed").d("reason", "Can't update media state. MediaStatus structure is wrong"));
        sendError("Can't update media state.");
        return;
    }

    // numeric parameters are sometimes converted to null during stringification, set these to 0
    const int trackIndex = getOptionalInt(state, TRACK_INDEX_KEY, 0);
    const int trackCount = getOptionalInt(state, TRACK_COUNT_KEY, 0);
    const int currentTime = getOptionalInt(state, CURRENT_TIME_KEY, 0);
    const int duration = getOptionalInt(state, DURATION_KEY, 0);

    const apl::MediaState mediaState(
        trackIndex, trackCount, currentTime, duration, state[PAUSED_KEY].GetBool(), state[ENDED_KEY].GetBool());
    component->updateMediaState(mediaState, fromEvent);
}

void AplCoreConnectionManager::handleGraphicUpdate(const rapidjson::Value& update) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleGraphicUpdateFailed").d("reason", "Root context is null"));
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        ACSDK_ERROR(LX("handleGraphicUpdateFailed").d("reason", "Unable to find component").d("id", id));
        sendError("Unable to find component");
        return;
    }

    auto json = apl::GraphicContent::create(update["avg"].GetString());
    component->updateGraphic(json);
}

void AplCoreConnectionManager::handleEnsureLayout(const rapidjson::Value& payload) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleEnsureLayoutFailed").d("reason", "Root context is null"));
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        ACSDK_ERROR(LX("handleEnsureLayoutFailed").d("reason", "Unable to find component").d("id", id));
        sendError("Unable to find component");
        return;
    }

    component->ensureLayout(true);
    auto msg = messages::AplCoreViewhostMessage(ENSURELAYOUT_KEY);
    send(msg.setPayload(id));
    runEventLoop();
}

void AplCoreConnectionManager::handleScrollToRectInComponent(const rapidjson::Value& payload) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleScrollToRectInComponentFailed").d("reason", "Root context is null"));
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        ACSDK_ERROR(LX("handleScrollToRectInComponent").d("reason", "Unable to find component").d("id", id));
        sendError("Unable to find component");
        return;
    }

    apl::Rect rect = convertJsonToScaledRect(payload);
    m_Root->scrollToRectInComponent(component, rect, static_cast<apl::CommandScrollAlign>(payload["align"].GetInt()));
}

void AplCoreConnectionManager::handleHandleKeyboard(const rapidjson::Value& payload) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleHandleKeyboardFailed").d("reason", "Root context is null"));
        return;
    }

    auto keyType = payload["keyType"].GetInt();
    auto code = payload["code"].GetString();
    auto key = payload["key"].GetString();
    auto repeat = payload["repeat"].GetBool();
    auto altKey = payload["altKey"].GetBool();
    auto ctrlKey = payload["ctrlKey"].GetBool();
    auto metaKey = payload["metaKey"].GetBool();
    auto shiftKey = payload["shiftKey"].GetBool();
    apl::Keyboard keyboard(code, key);
    keyboard.repeat(repeat);
    keyboard.alt(altKey);
    keyboard.ctrl(ctrlKey);
    keyboard.meta(metaKey);
    keyboard.shift(shiftKey);
    m_Root->handleKeyboard(static_cast<apl::KeyHandlerType>(keyType), keyboard);
}

void AplCoreConnectionManager::handleUpdateCursorPosition(const rapidjson::Value& payload) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleUpdateCursorPositionFailed").d("reason", "Root context is null"));
        return;
    }

    const float x = payload[X_KEY].GetFloat();
    const float y = payload[Y_KEY].GetFloat();
    apl::Point cursorPosition(m_AplCoreMetrics->toCore(x), m_AplCoreMetrics->toCore(y));
    m_Root->updateCursorPosition(cursorPosition);
}

void AplCoreConnectionManager::handleEventResponse(const rapidjson::Value& response) {
    if (!m_Root) {
        ACSDK_ERROR(LX("handleEventResponseFailed").d("reason", "Root context is null"));
        return;
    }

    if (!response["event"].IsInt()) {
        ACSDK_ERROR(LX("handleEventResponseFailed").d("reason", "Invalid event response"));
        sendError("Invalid event response");
        return;
    }
    auto event = response["event"].GetInt();
    auto it = m_PendingEvents.find(event);
    if (it != m_PendingEvents.end()) {
        auto rectJson = response.FindMember("rectArgument");
        if (rectJson != response.MemberEnd()) {
            apl::Rect rect = convertJsonToScaledRect(rectJson->value);
            it->second.resolve(rect);
        } else {
            auto arg = response.FindMember("argument");
            if (arg != response.MemberEnd()) {
                it->second.resolve(arg->value.GetInt());
            } else {
                it->second.resolve();
            }
        }
        m_PendingEvents.erase(it);
    }

    runEventLoop();
}

unsigned int AplCoreConnectionManager::send(messages::AplCoreViewhostMessage& message) {
    unsigned int seqno = ++m_SequenceNumber;
    auto aplCoreMessage = messages::AplCoreMessage(message.setSequenceNumber(seqno).getValue());
    m_guiClientInterface->sendMessage(aplCoreMessage);
    return seqno;
}

rapidjson::Document AplCoreConnectionManager::blockingSend(
    messages::AplCoreViewhostMessage& message,
    const std::chrono::milliseconds& timeout) {
    std::lock_guard<std::mutex> lock{m_blockingSendMutex};
    m_replyPromise = std::promise<std::string>();
    m_replyExpectedSequenceNumber = send(message);
    m_blockingSendReplyExpected = true;

    auto future = m_replyPromise.get_future();
    auto status = future.wait_for(timeout);
    if (status != std::future_status::ready) {
        m_blockingSendReplyExpected = false;
        ACSDK_ERROR(LX("blockingSendFailed").d("reason", "Did not receive response"));
        return rapidjson::Document(rapidjson::kNullType);
    }

    rapidjson::Document doc;
    if (!jsonUtils::parseJSON(future.get(), &doc)) {
        ACSDK_ERROR(LX("blockingSendFailed").d("reason", "parsingFailed"));
        return rapidjson::Document(rapidjson::kNullType);
        ;
    }

    return doc;
}

void AplCoreConnectionManager::sendError(const std::string& message) {
    auto reply = messages::AplCoreViewhostMessage(ERROR_KEY);
    send(reply.setPayload(message));
}

void AplCoreConnectionManager::runEventLoop() {
    while (m_Root->hasEvent()) {
        // Check for screen lock
        if (m_Root->screenLock() && !m_ScreenLock) {
            m_guiManager->handleActivityEvent(APL_SCREEN_LOCK, smartScreenSDKInterfaces::ActivityEvent::ACTIVATED);
            m_ScreenLock = true;
        } else if (!m_Root->screenLock() && m_ScreenLock) {
            m_guiManager->handleActivityEvent(APL_SCREEN_LOCK, smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
            m_ScreenLock = false;
        }

        // Generate an event and send it up
        auto event = m_Root->popEvent();

        if (apl::EventType::kEventTypeSendEvent == event.getType()) {
            rapidjson::Document userEventPayloadJson(rapidjson::kObjectType);
            auto& allocator = userEventPayloadJson.GetAllocator();
            auto source = event.getValue(apl::EventProperty::kEventPropertySource);
            auto components = event.getValue(apl::EventProperty::kEventPropertyComponents);
            auto arguments = event.getValue(apl::EventProperty::kEventPropertyArguments);

            userEventPayloadJson.AddMember(
                PRESENTATION_TOKEN_KEY, rapidjson::Value(m_aplToken.c_str(), allocator).Move(), allocator);
            userEventPayloadJson.AddMember(SOURCE_KEY, source.serialize(allocator).Move(), allocator);
            userEventPayloadJson.AddMember(ARGUMENTS_KEY, arguments.serialize(allocator).Move(), allocator);
            userEventPayloadJson.AddMember(COMPONENTS_KEY, components.serialize(allocator).Move(), allocator);

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            userEventPayloadJson.Accept(writer);

            m_guiManager->handleUserEvent(sb.GetString());
            continue;
        }

        auto msg = messages::AplCoreViewhostMessage(EVENT_KEY);
        auto token = send(msg.setPayload(event.serialize(msg.alloc())));

        // If the event had an action ref, stash the reference for future use
        auto ref = event.getActionRef();
        if (!ref.isEmpty()) {
            m_PendingEvents.emplace(token, ref);
            ref.addTerminateCallback([this, token](const std::shared_ptr<apl::Timers>&) {
                auto it = m_PendingEvents.find(token);
                if (it != m_PendingEvents.end()) {
                    m_PendingEvents.erase(it);  // Remove the pending event

                    auto msg = messages::AplCoreViewhostMessage(EVENT_TERMINATE_KEY);
                    rapidjson::Value payload(rapidjson::kObjectType);
                    payload.AddMember("token", token, msg.alloc());
                    send(msg.setPayload(std::move(payload)));
                } else {
                    ACSDK_WARN(LX("runEventLoopFailedInTerminateCallback").d("reason", "Event was not pending"));
                }
            });
        }
    }

    // Send up all of the dirty properties
    auto& dirty = m_Root->getDirty();
    if (dirty.size()) {
        auto msg = messages::AplCoreViewhostMessage(DIRTY_KEY);
        rapidjson::Value array(rapidjson::kArrayType);
        for (auto& component : dirty) array.PushBack(component->serializeDirty(msg.alloc()), msg.alloc());
        send(msg.setPayload(std::move(array)));

        m_Root->clearDirty();
    }
}

void AplCoreConnectionManager::onConnectionOpened() {
    ACSDK_DEBUG5(LX("onConnectionOpened"));
    // Start the scheduled event timer to refresh the display at 60fps
    m_executor.submit([this] {
        m_updateTimer.start(
            std::chrono::milliseconds(16),
            Timer::PeriodType::ABSOLUTE,
            Timer::FOREVER,
            std::bind(&AplCoreConnectionManager::onUpdateTimer, this));
    });
}

void AplCoreConnectionManager::onConnectionClosed() {
    ACSDK_DEBUG5(LX("onConnectionClosed"));
    // Stop the outstanding timer as the client is no longer connected
    m_executor.submit([this] { m_updateTimer.stop(); });
}

void AplCoreConnectionManager::onUpdateTimer() {
    m_executor.submit([this] {
        if (m_Root) {
            auto now = getCurrentTime() - m_StartTime;
            m_Root->updateTime(now.count());
            runEventLoop();
        }
    });
}

apl::Rect AplCoreConnectionManager::convertJsonToScaledRect(const rapidjson::Value& jsonNode) {
    const float scale = m_AplCoreMetrics->toCore(1.0f);
    const float x = jsonNode[X_KEY].IsNumber() ? jsonNode[X_KEY].GetFloat() : 0.0f;
    const float y = jsonNode[Y_KEY].IsNumber() ? jsonNode[Y_KEY].GetFloat() : 0.0f;
    const float width = jsonNode[WIDTH_KEY].IsNumber() ? jsonNode[WIDTH_KEY].GetFloat() : 0.0f;
    const float height = jsonNode[HEIGHT_KEY].IsNumber() ? jsonNode[HEIGHT_KEY].GetFloat() : 0.0f;

    return apl::Rect(x * scale, y * scale, width * scale, height * scale);
}

void AplCoreConnectionManager::setGuiManager(
    const std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> guiManager) {
    m_guiManager = guiManager;
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
