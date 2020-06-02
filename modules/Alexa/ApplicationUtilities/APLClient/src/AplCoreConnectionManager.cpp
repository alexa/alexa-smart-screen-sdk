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

#include "APLClient/AplCoreTextMeasurement.h"
#include "APLClient/AplCoreConnectionManager.h"
#include "APLClient/AplCoreViewhostMessage.h"

#include <apl/datasource/dynamicindexlistdatasourceprovider.h>

namespace APLClient {

/// The keys used in ProvideState.
static const char TOKEN_KEY[] = "token";
static const char VERSION_KEY[] = "version";
static const char CONTEXT_KEY[] = "componentsVisibleOnScreen";
/// The value used in ProvideState.
// TODO: need to get version number from APLCoreEngine: ARC-858
static const char VERSION_VALUE[] = "AplRenderer-1.2";

// Key used in messaging
static const char SEQNO_KEY[] = "seqno";

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
static const char BACKGROUND_KEY[] = "background";
static const char SCREENLOCK_KEY[] = "screenLock";
static const char COLOR_KEY[] = "color";
static const char GRADIENT_KEY[] = "gradient";
static const char ENSURELAYOUT_KEY[] = "ensureLayout";
static const char AGENTNAME_KEY[] = "agentName";
static const char AGENTVERSION_KEY[] = "agentVersion";
static const char ALLOWOPENURL_KEY[] = "allowOpenUrl";
static const char DISALLOWVIDEO_KEY[] = "disallowVideo";
static const char ANIMATIONQUALITY_KEY[] = "animationQuality";

/// The keys used in APL event execution.
static const char ERROR_KEY[] = "error";
static const char EVENT_KEY[] = "event";
static const char EVENT_TERMINATE_KEY[] = "eventTerminate";
static const char DIRTY_KEY[] = "dirty";

/// SendEvent keys
static const char PRESENTATION_TOKEN_KEY[] = "presentationToken";
static const char SOURCE_KEY[] = "source";
static const char ARGUMENTS_KEY[] = "arguments";
static const char COMPONENTS_KEY[] = "components";

/// RuntimeError keys
static const char ERRORS_KEY[] = "errors";

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

AplCoreConnectionManager::AplCoreConnectionManager(const AplOptionsInterfacePtr aplOptions) :
        m_aplOptions{aplOptions},
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

void AplCoreConnectionManager::setContent(const apl::ContentPtr content, const std::string& token) {
    m_Content = content;
    m_aplToken = token;
    m_aplOptions->resetViewhost(token);
}

void AplCoreConnectionManager::setSupportedViewports(const std::string& jsonPayload) {
    rapidjson::Document doc;
    if (doc.Parse(jsonPayload.c_str()).HasParseError()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "setSupportedViewportsFailed", "Failed to parse json payload");
        return;
    }

    if (doc.GetType() != rapidjson::Type::kArrayType) {
        m_aplOptions->logMessage(LogLevel::ERROR, "setSupportedViewportsFailed", "Unexpected json document type");
        return;
    }

    m_ViewportSizeSpecifications.clear();
    for (auto& spec : doc.GetArray()) {
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

bool AplCoreConnectionManager::shouldHandleMessage(const std::string& message) {
    if (m_blockingSendReplyExpected) {
        rapidjson::Document doc;
        if (doc.Parse(message.c_str()).HasParseError()) {
            m_aplOptions->logMessage(LogLevel::ERROR, "shouldHandleMessageFailed", "Error whilst parsing message");
            return false;
        }

        if (doc.HasMember(SEQNO_KEY) && doc[SEQNO_KEY].IsNumber()) {
            unsigned int seqno = doc[SEQNO_KEY].GetUint();
            if (seqno == m_replyExpectedSequenceNumber) {
                m_blockingSendReplyExpected = false;
                m_replyPromise.set_value(message);
                return false;
            }
        }
    }

    return true;
}

void AplCoreConnectionManager::handleMessage(const std::string& message) {
    rapidjson::Document doc;
    if (doc.Parse(message.c_str()).HasParseError()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Error whilst parsing message");
        return;
    }

    if (!doc.HasMember("type")) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unable to find type in message");
        return;
    }
    std::string type = doc["type"].GetString();

    auto payload = doc.FindMember("payload");
    if (payload == doc.MemberEnd()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unable to find payload in message");
        return;
    }

    auto fit = m_messageHandlers.find(type);
    if (fit != m_messageHandlers.end()) {
        fit->second(payload->value);
    } else {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unrecognized message type: " + type);
    }
}

void AplCoreConnectionManager::executeCommands(const std::string& command, const std::string& token) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Root context is missing");
        return;
    }

    rapidjson::Document* document = new rapidjson::Document();
    if (document->Parse(command).HasParseError()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Parse commands failed");
        return;
    }

    auto it = document->FindMember("commands");
    if (it == document->MemberEnd() || it->value.GetType() != rapidjson::Type::kArrayType) {
        m_aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Missing commands, or is not array");
        return;
    }

    apl::Object object{it->value};
    auto action = m_Root->executeCommands(object, false);
    if (!action) {
        m_aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Execute commands failed");
        return;
    }

    m_aplOptions->onActivityStarted(APL_COMMAND_EXECUTION);

    action->setUserData(document);

    action->then([this, document, token](const apl::ActionPtr& action) {
        m_aplOptions->logMessage(LogLevel::DBG, "executeCommands", "Command sequence complete");
        if (action->getUserData()) {
            delete document;
            action->setUserData(nullptr);
        }
        m_aplOptions->onCommandExecutionComplete(token, true);

        m_aplOptions->onActivityEnded(APL_COMMAND_EXECUTION);
    });
    action->addTerminateCallback([this, action, document, token](const apl::TimersPtr&) {
        m_aplOptions->logMessage(LogLevel::DBG, "executeCommandsFailed", "Command sequence failed");
        if (action->getUserData()) {
            delete document;
            action->setUserData(nullptr);
        }
        m_aplOptions->onCommandExecutionComplete(token, false);

        m_aplOptions->onActivityEnded(APL_COMMAND_EXECUTION);
    });
}

void AplCoreConnectionManager::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Root context is missing");
        return;
    }

    auto provider = m_Root->context().getRootConfig().getDataSourceProvider(sourceType);
    if (!provider) {
        m_aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Unknown provider requested.");
        return;
    }

    bool result = provider->processUpdate(jsonPayload);
    if (!result) {
        m_aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Update is not processed.");
        checkAndSendDataSourceErrors();
    }
}

void AplCoreConnectionManager::provideState(unsigned int stateRequestToken) {
    if (!m_Content) {
        m_aplOptions->logMessage(LogLevel::WARN, "provideStateFailed", "Root context is null");
        sendError("Root context is null");
        return;
    }

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
        m_aplOptions->logMessage(LogLevel::ERROR, "provideStateFailed", "Unable to get visual context");
        rapidjson::Value emptyObj(rapidjson::kObjectType);
        // add an empty visual context
        arr.PushBack(emptyObj, allocator);
    }
    // Add visual context info
    state.AddMember(CONTEXT_KEY, arr, allocator);
    state.Accept(writer);
    m_aplOptions->onVisualContextAvailable(stateRequestToken, buffer.GetString());
}

void AplCoreConnectionManager::interruptCommandSequence() {
    if (m_Root) {
        m_Root->cancelExecution();
    }
}

void AplCoreConnectionManager::handleBuild(const rapidjson::Value& message) {
    /* APL Document Inflation started */
    m_aplOptions->onRenderingEvent(AplRenderingEvent::INFLATE_BEGIN);

    auto renderingOptionsMsg = AplCoreViewhostMessage(RENDERING_OPTIONS_KEY);
    rapidjson::Value renderingOptions(rapidjson::kObjectType);
    renderingOptions.AddMember(LEGACY_KARAOKE_KEY, m_Content->getAPLVersion() == "1.0", renderingOptionsMsg.alloc());
    send(renderingOptionsMsg.setPayload(std::move(renderingOptions)));

    if (!m_Content) {
        m_aplOptions->logMessage(LogLevel::WARN, "handleBuildFailed", "No content to build");
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
                      .measure(std::make_shared<AplCoreTextMeasurement>(shared_from_this(), m_aplOptions))
                      .utcTime(getCurrentTime().count())
                      .localTimeAdjustment(m_aplOptions->getTimezoneOffset().count())
                      .enforceAPLVersion(apl::APLVersion::kAPLVersionIgnore)
                      .sequenceChildCache(5);

    // Data Sources
    config.dataSourceProvider(
        apl::DynamicIndexListConstants::DEFAULT_TYPE_NAME, std::make_shared<apl::DynamicIndexListDataSourceProvider>());

    m_PendingEvents.clear();

    // Release the activity tracker
    m_aplOptions->onActivityEnded(APL_COMMAND_EXECUTION);

    if (m_ScreenLock) {
        m_aplOptions->onActivityEnded(APL_SCREEN_LOCK);
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
            m_AplCoreMetrics = std::make_shared<AplCoreMetrics>(m_Metrics, scalingOptions);
        } else {
            m_AplCoreMetrics = std::make_shared<AplCoreMetrics>(m_Metrics);
        }

        // Send scaling metrics out to viewhost
        auto reply = AplCoreViewhostMessage(SCALING_KEY);
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
            m_aplOptions->logMessage(
                LogLevel::WARN, __func__, "Unable to inflate document with current chosen scaling.");
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

    /* APL Core Inflation ended */
    m_aplOptions->onRenderingEvent(AplRenderingEvent::INFLATE_END);

    if (m_Root) {
        sendDocumentThemeMessage();

        sendDocumentBackgroundMessage(m_Content->getBackground(m_AplCoreMetrics->getMetrics(), config));

        auto reply = AplCoreViewhostMessage(HIERARCHY_KEY);
        send(reply.setPayload(m_Root->topComponent()->serialize(reply.alloc())));

        auto idleTimeout = std::chrono::milliseconds(m_Root->settings().idleTimeout());
        m_aplOptions->onSetDocumentIdleTimeout(idleTimeout);
        m_aplOptions->onRenderDocumentComplete(m_aplToken, true, "");
    } else {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleBuildFailed", "Unable to inflate document");
        sendError("Unable to inflate document");
        m_aplOptions->onRenderDocumentComplete(m_aplToken, false, "Unable to inflate document");
        // Send DataSource errors if any
        checkAndSendDataSourceErrors();
    }
}

void AplCoreConnectionManager::sendDocumentThemeMessage() {
    if (m_Root) {
        auto themeMsg = AplCoreViewhostMessage(DOCTHEME_KEY);
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

void AplCoreConnectionManager::sendDocumentBackgroundMessage(const apl::Object& background) {
    auto backgroundMsg = AplCoreViewhostMessage(BACKGROUND_KEY);
    auto& alloc = backgroundMsg.alloc();
    rapidjson::Value payload(rapidjson::kObjectType);
    rapidjson::Value backgroundValue(rapidjson::kObjectType);
    if (background.isColor()) {
        backgroundValue.AddMember(COLOR_KEY, background.asString(), alloc);
    } else if (background.isGradient()) {
        backgroundValue.AddMember(GRADIENT_KEY, background.getGradient().serialize(alloc), alloc);
    } else {
        backgroundValue.AddMember(COLOR_KEY, apl::Color().asString(), alloc);
    }
    payload.AddMember(BACKGROUND_KEY, backgroundValue, alloc);
    backgroundMsg.setPayload(std::move(payload));
    send(backgroundMsg);
}

void AplCoreConnectionManager::sendScreenLockMessage(bool screenLock) {
    auto screenLockMsg = AplCoreViewhostMessage(SCREENLOCK_KEY);
    auto& alloc = screenLockMsg.alloc();
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember(SCREENLOCK_KEY, screenLock, alloc);
    screenLockMsg.setPayload(std::move(payload));
    send(screenLockMsg);
}

void AplCoreConnectionManager::handleUpdate(const rapidjson::Value& update) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleUpdateFailed", std::string("Unable to find component with id: ") + id);
        sendError("Unable to find component");
        return;
    }

    auto type = static_cast<apl::UpdateType>(update["type"].GetInt());
    auto value = update["value"].GetFloat();
    if (type == apl::UpdateType::kUpdateScrollPosition) {
        value = m_AplCoreMetrics->toCore(value);
    }

    component->update(type, value);
}

void AplCoreConnectionManager::handleMediaUpdate(const rapidjson::Value& update) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleMediaUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleMediaUpdateFailed", std::string("Unable to find component with id: ") + id);
        sendError("Unable to find component");
        return;
    }

    if (!update.HasMember(MEDIA_STATE_KEY) || !update.HasMember(FROM_EVENT_KEY)) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleMediaUpdateFailed", "State update object is missing parameters");
        sendError("Can't update media state.");
        return;
    }
    auto& state = update[MEDIA_STATE_KEY];
    auto fromEvent = update[FROM_EVENT_KEY].GetBool();

    if (!state.HasMember(TRACK_INDEX_KEY) || !state.HasMember(TRACK_COUNT_KEY) || !state.HasMember(CURRENT_TIME_KEY) ||
        !state.HasMember(DURATION_KEY) || !state.HasMember(PAUSED_KEY) || !state.HasMember(ENDED_KEY)) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleMediaUpdateFailed", "Can't update media state. MediaStatus structure is wrong");
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
        m_aplOptions->logMessage(LogLevel::ERROR, "handleGraphicUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleGraphicUpdateFailed", std::string("Unable to find component with id:") + id);
        sendError("Unable to find component");
        return;
    }

    auto json = apl::GraphicContent::create(update["avg"].GetString());
    component->updateGraphic(json);
}

void AplCoreConnectionManager::handleEnsureLayout(const rapidjson::Value& payload) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleEnsureLayoutFailed", "Root context is null");
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        m_aplOptions->logMessage(
            LogLevel::ERROR, "handleEnsureLayoutFailed", std::string("Unable to find component with id:") + id);
        sendError("Unable to find component");
        return;
    }

    component->ensureLayout(true);
    auto msg = AplCoreViewhostMessage(ENSURELAYOUT_KEY);
    send(msg.setPayload(id));
}

void AplCoreConnectionManager::handleScrollToRectInComponent(const rapidjson::Value& payload) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleScrollToRectInComponentFailed", "Root context is null");
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->context().findComponentById(id);
    if (!component) {
        m_aplOptions->logMessage(
            LogLevel::ERROR,
            "handleScrollToRectInComponentFailed",
            std::string("Unable to find component with id:") + id);
        sendError("Unable to find component");
        return;
    }

    apl::Rect rect = convertJsonToScaledRect(payload);
    m_Root->scrollToRectInComponent(component, rect, static_cast<apl::CommandScrollAlign>(payload["align"].GetInt()));
}

void AplCoreConnectionManager::handleHandleKeyboard(const rapidjson::Value& payload) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleHandleKeyboardFailed", "Root context is null");
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
        m_aplOptions->logMessage(LogLevel::ERROR, "handleUpdateCursorPositionFailed", "Root context is null");
        return;
    }

    const float x = payload[X_KEY].GetFloat();
    const float y = payload[Y_KEY].GetFloat();
    apl::Point cursorPosition(m_AplCoreMetrics->toCore(x), m_AplCoreMetrics->toCore(y));
    m_Root->updateCursorPosition(cursorPosition);
}

void AplCoreConnectionManager::handleEventResponse(const rapidjson::Value& response) {
    if (!m_Root) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleEventResponseFailed", "Root context is null");
        return;
    }

    if (!response["event"].IsInt()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "handleEventResponseFailed", "Invalid event response");
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
}

unsigned int AplCoreConnectionManager::send(AplCoreViewhostMessage& message) {
    unsigned int seqno = ++m_SequenceNumber;
    m_aplOptions->sendMessage(message.setSequenceNumber(seqno).get());
    return seqno;
}

rapidjson::Document AplCoreConnectionManager::blockingSend(
    AplCoreViewhostMessage& message,
    const std::chrono::milliseconds& timeout) {
    std::lock_guard<std::mutex> lock{m_blockingSendMutex};
    m_replyPromise = std::promise<std::string>();
    m_replyExpectedSequenceNumber = send(message);
    m_blockingSendReplyExpected = true;

    auto future = m_replyPromise.get_future();
    auto status = future.wait_for(timeout);
    if (status != std::future_status::ready) {
        m_blockingSendReplyExpected = false;
        // Under the situation that finish command destroys the renderer, there is no response.
        m_aplOptions->logMessage(LogLevel::WARN, "blockingSendFailed", "Did not receive response");
        return rapidjson::Document(rapidjson::kNullType);
    }

    rapidjson::Document doc;
    if (doc.Parse(future.get()).HasParseError()) {
        m_aplOptions->logMessage(LogLevel::ERROR, "blockingSendFailed", "parsingFailed");
        return rapidjson::Document(rapidjson::kNullType);
    }

    return doc;
}

void AplCoreConnectionManager::sendError(const std::string& message) {
    auto reply = AplCoreViewhostMessage(ERROR_KEY);
    send(reply.setPayload(message));
}

void AplCoreConnectionManager::handleScreenLock() {
    if (m_Root->screenLock() && !m_ScreenLock) {
        m_aplOptions->onActivityStarted(APL_SCREEN_LOCK);
        m_ScreenLock = true;
    } else if (!m_Root->screenLock() && m_ScreenLock) {
        m_aplOptions->onActivityEnded(APL_SCREEN_LOCK);
        m_ScreenLock = false;
    } else {
        return;
    }
    sendScreenLockMessage(m_ScreenLock);
}

void AplCoreConnectionManager::processEvent(const apl::Event& event) {
    if (apl::EventType::kEventTypeFinish == event.getType()) {
        m_aplOptions->onFinish();
        return;
    }

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

        m_aplOptions->onSendEvent(sb.GetString());
        return;
    }

    if (apl::EventType::kEventTypeDataSourceFetchRequest == event.getType()) {
        rapidjson::Document fetchRequestPayloadJson(rapidjson::kObjectType);
        auto& allocator = fetchRequestPayloadJson.GetAllocator();
        auto type = event.getValue(apl::EventProperty::kEventPropertyName);
        auto payload = event.getValue(apl::EventProperty::kEventPropertyValue);

        apl::ObjectMap fetchRequest(payload.getMap());
        fetchRequest.emplace(PRESENTATION_TOKEN_KEY, m_aplToken);

        auto fetch = apl::Object(std::make_shared<apl::ObjectMap>(fetchRequest)).serialize(allocator);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        fetch.Accept(writer);

        m_aplOptions->onDataSourceFetchRequestEvent(type.asString(), sb.GetString());
        return;
    }

    auto msg = AplCoreViewhostMessage(EVENT_KEY);
    auto token = send(msg.setPayload(event.serialize(msg.alloc())));

    // If the event had an action ref, stash the reference for future use
    auto ref = event.getActionRef();
    if (!ref.isEmpty()) {
        m_PendingEvents.emplace(token, ref);
        ref.addTerminateCallback([this, token](const apl::TimersPtr&) {
            auto it = m_PendingEvents.find(token);
            if (it != m_PendingEvents.end()) {
                m_PendingEvents.erase(it);  // Remove the pending event

                auto msg = AplCoreViewhostMessage(EVENT_TERMINATE_KEY);
                rapidjson::Value payload(rapidjson::kObjectType);
                payload.AddMember("token", token, msg.alloc());
                send(msg.setPayload(std::move(payload)));
            } else {
                m_aplOptions->logMessage(LogLevel::WARN, __func__, "Event was not pending");
            }
        });
    }
}

void AplCoreConnectionManager::processDirty(const std::set<apl::ComponentPtr>& dirty) {
    std::map<std::string, rapidjson::Value> tempDirty;
    auto msg = AplCoreViewhostMessage(DIRTY_KEY);

    for (auto& component : dirty) {
        if (component->getDirty().count(apl::kPropertyNotifyChildrenChanged)) {
            auto notify = component->getCalculated(apl::kPropertyNotifyChildrenChanged);
            const auto& changed = notify.getArray();
            // Whenever we get NotifyChildrenChanged we get 2 types of action
            // Either insert or delete. The delete will happen on the viewhost level
            // However, insert needs the full serialized component from core & will be initalized
            // on apl-client side
            for (size_t i = 0; i < changed.size(); i++) {
                auto newChildId = changed.at(i).get("uid").asString();
                auto newChildIndex = changed.at(i).get("index").asInt();
                auto action = changed.at(i).get("action").asString();
                if (action == "insert") {
                    tempDirty[newChildId] = component->getChildAt(newChildIndex)->serialize(msg.alloc());
                }
            }
        }
        if (tempDirty.find(component->getUniqueId()) == tempDirty.end()) {
            tempDirty.emplace(component->getUniqueId(), component->serializeDirty(msg.alloc()));
        }
    }

    rapidjson::Value array(rapidjson::kArrayType);
    for (auto rit = tempDirty.rbegin(); rit != tempDirty.rend(); rit++) {
        auto uid = rit->first;
        auto& update = rit->second;

        array.PushBack(update.Move(), msg.alloc());
    }
    send(msg.setPayload(std::move(array)));
}

void AplCoreConnectionManager::coreFrameUpdate() {
    auto now = getCurrentTime() - m_StartTime;
    m_Root->updateTime(now.count(), getCurrentTime().count());
    m_Root->setLocalTimeAdjustment(m_aplOptions->getTimezoneOffset().count());

    m_Root->clearPending();

    while (m_Root->hasEvent()) {
        processEvent(m_Root->popEvent());
    }

    if (m_Root->isDirty()) {
        processDirty(m_Root->getDirty());
        m_Root->clearDirty();
    }

    handleScreenLock();
}

void AplCoreConnectionManager::onUpdateTick() {
    if (m_Root) {
        coreFrameUpdate();
        // Check regularly as something like timed-out fetch requests could come up.
        checkAndSendDataSourceErrors();
    }
}

apl::Rect AplCoreConnectionManager::convertJsonToScaledRect(const rapidjson::Value& jsonNode) {
    const float scale = m_AplCoreMetrics->toCore(1.0f);
    const float x = jsonNode[X_KEY].IsNumber() ? jsonNode[X_KEY].GetFloat() : 0.0f;
    const float y = jsonNode[Y_KEY].IsNumber() ? jsonNode[Y_KEY].GetFloat() : 0.0f;
    const float width = jsonNode[WIDTH_KEY].IsNumber() ? jsonNode[WIDTH_KEY].GetFloat() : 0.0f;
    const float height = jsonNode[HEIGHT_KEY].IsNumber() ? jsonNode[HEIGHT_KEY].GetFloat() : 0.0f;

    return apl::Rect(x * scale, y * scale, width * scale, height * scale);
}

void AplCoreConnectionManager::checkAndSendDataSourceErrors() {
    // TODO: Single provider supported as of now.
    auto provider =
        m_Root->context().getRootConfig().getDataSourceProvider(apl::DynamicIndexListConstants::DEFAULT_TYPE_NAME);
    if (provider) {
        auto errors = provider->getPendingErrors();
        if (!errors.empty() && errors.isArray()) {
            auto errorEvent = std::make_shared<apl::ObjectMap>();
            errorEvent->emplace(PRESENTATION_TOKEN_KEY, m_aplToken);
            errorEvent->emplace(ERRORS_KEY, errors);

            rapidjson::Document runtimeErrorPayloadJson(rapidjson::kObjectType);
            auto& allocator = runtimeErrorPayloadJson.GetAllocator();
            auto runtimeError = apl::Object(errorEvent).serialize(allocator);

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
            runtimeError.Accept(writer);

            m_aplOptions->onRuntimeErrorEvent(sb.GetString());
        }
    }
}

void AplCoreConnectionManager::reset() {
    m_aplToken = "";
    m_Root.reset();
    m_Content.reset();
}

}  // namespace APLClient
