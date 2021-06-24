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

#include <climits>

#include "APLClient/AplCoreTextMeasurement.h"
#include "APLClient/AplCoreLocaleMethods.h"
#include "APLClient/AplCoreConnectionManager.h"
#include "APLClient/AplCoreViewhostMessage.h"

#include <apl/datasource/dynamicindexlistdatasourceprovider.h>
#include <apl/datasource/dynamictokenlistdatasourceprovider.h>

namespace APLClient {

/// The keys used in ProvideState.
static const char TOKEN_KEY[] = "token";
static const char VERSION_KEY[] = "version";
static const char CONTEXT_KEY[] = "componentsVisibleOnScreen";
/// The value used in ProvideState.
static const char VERSION_VALUE[] = "AplRenderer-1.4";

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
static const char REHIERARCHY_KEY[] = "reHierarchy";
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
static const char SUPPORTED_EXTENSIONS[] = "supportedExtensions";

/// The keys used in OS accessibility settings.
static const char FONTSCALE_KEY[] = "fontScale";
static const char SCREENMODE_KEY[] = "screenMode";
static const char SCREENREADER_KEY[] = "screenReader";

/// Document settings keys.
static const char SUPPORTS_RESIZING_KEY[] = "supportsResizing";

/// The keys used in APL event execution.
static const char ERROR_KEY[] = "error";
static const char EVENT_KEY[] = "event";
static const char ARGUMENT_KEY[] = "argument";
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

/// HandlePointerEvent keys
static const char POINTEREVENTTYPE_KEY[] = "pointerEventType";
static const char POINTERTYPE_KEY[] = "pointerType";
static const char POINTERID_KEY[] = "pointerId";

/// Data sources
static const std::vector<std::string> KNOWN_DATA_SOURCES = {
    apl::DynamicIndexListConstants::DEFAULT_TYPE_NAME,
    apl::DynamicTokenListConstants::DEFAULT_TYPE_NAME,
};

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

static apl::Bimap<std::string, apl::RootConfig::ScreenMode> AVS_SCREEN_MODE_MAP = {
        {"normal", apl::RootConfig::kScreenModeNormal},
        {"high-contrast", apl::RootConfig::kScreenModeHighContrast},
};

AplCoreConnectionManager::AplCoreConnectionManager(AplConfigurationPtr config) :
        m_aplConfiguration{config},
        m_ScreenLock{false},
        m_SequenceNumber{0},
        m_replyExpectedSequenceNumber{0},
        m_blockingSendReplyExpected{false} {
    m_StartTime = getCurrentTime();
    m_renderingStart = std::chrono::steady_clock::time_point(std::chrono::milliseconds(0));

    m_extensionManager = std::make_shared<AplCoreExtensionManager>();
    m_messageHandlers.emplace("build", [this](const rapidjson::Value& payload) { handleBuild(payload); });
    m_messageHandlers.emplace("configurationChange", [this](const rapidjson::Value& payload) { handleConfigurationChange(payload); });
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
            "getFocusableAreas", [this](const rapidjson::Value& payload) { getFocusableAreas(payload); });
    m_messageHandlers.emplace(
            "getFocused", [this](const rapidjson::Value& payload) { getFocused(payload); });
    m_messageHandlers.emplace(
            "setFocus", [this](const rapidjson::Value& payload) { setFocus(payload); });
    m_messageHandlers.emplace(
        "updateCursorPosition", [this](const rapidjson::Value& payload) { handleUpdateCursorPosition(payload); });
    m_messageHandlers.emplace(
        "handlePointerEvent", [this](const rapidjson::Value& payload) { handleHandlePointerEvent(payload); });
    m_messageHandlers.emplace(
        "isCharacterValid", [this](const rapidjson::Value& payload) { handleIsCharacterValid(payload); });
    m_messageHandlers.emplace("reInflate", [this](const rapidjson::Value& payload) { handleReInflate(payload); });
    m_messageHandlers.emplace("reHierarchy", [this](const rapidjson::Value& payload) { handleReHierarchy(payload); });
    m_messageHandlers.emplace("getDisplayedChildCount", [this](const rapidjson::Value& payload) { handleGetDisplayedChildCount(payload); });
    m_messageHandlers.emplace("getDisplayedChildId", [this](const rapidjson::Value& payload) { handleGetDisplayedChildId(payload); });
}

void AplCoreConnectionManager::setContent(const apl::ContentPtr content, const std::string& token) {
    m_Content = content;
    m_aplToken = token;
    m_ConfigurationChange.clear();
    m_aplConfiguration->getAplOptions()->resetViewhost(token);
}

void AplCoreConnectionManager::setSupportedViewports(const std::string& jsonPayload) {
    rapidjson::Document doc;
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (doc.Parse(jsonPayload.c_str()).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "setSupportedViewportsFailed", "Failed to parse json payload");
        return;
    }

    if (doc.GetType() != rapidjson::Type::kArrayType) {
        aplOptions->logMessage(LogLevel::ERROR, "setSupportedViewportsFailed", "Unexpected json document type");
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
            auto aplOptions = m_aplConfiguration->getAplOptions();
            aplOptions->logMessage(LogLevel::ERROR, "shouldHandleMessageFailed", "Error whilst parsing message");
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
    auto aplOptions = m_aplConfiguration->getAplOptions();
    rapidjson::Document doc;
    if (doc.Parse(message.c_str()).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Error whilst parsing message");
        return;
    }

    if (!doc.HasMember("type")) {
        aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unable to find type in message");
        return;
    }
    std::string type = doc["type"].GetString();

    auto payload = doc.FindMember("payload");
    if (payload == doc.MemberEnd()) {
        aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unable to find payload in message");
        return;
    }

    auto fit = m_messageHandlers.find(type);
    if (fit != m_messageHandlers.end()) {
        fit->second(payload->value);
    } else {
        aplOptions->logMessage(LogLevel::ERROR, "handleMessageFailed", "Unrecognized message type: " + type);
    }
}

void AplCoreConnectionManager::handleConfigurationChange(const rapidjson::Value& configurationChange) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root || !m_AplCoreMetrics) {
        aplOptions->logMessage(LogLevel::ERROR, "handleConfigurationChangeFailed", "Root context is missing");
        return;
    }

    apl::ConfigurationChange configChange = apl::ConfigurationChange();
    // config change for width and height
    if (configurationChange.HasMember(WIDTH_KEY) && configurationChange[WIDTH_KEY].IsInt() && configurationChange.HasMember(HEIGHT_KEY) && configurationChange[HEIGHT_KEY].IsInt()) {

        m_Metrics.size((int) configurationChange[WIDTH_KEY].GetInt(), (int) configurationChange[HEIGHT_KEY].GetInt());
        m_AplCoreMetrics.reset();
        apl::ScalingOptions scalingOptions = {
                m_ViewportSizeSpecifications, SCALING_BIAS_CONSTANT, SCALING_SHAPE_OVERRIDES_COST};
        if (!scalingOptions.getSpecifications().empty()) {
            m_AplCoreMetrics = std::make_shared<AplCoreMetrics>(m_Metrics, scalingOptions);
        } else {
            m_AplCoreMetrics = std::make_shared<AplCoreMetrics>(m_Metrics);
        }

        const int pixelWidth = (int) m_AplCoreMetrics->toCorePixel(m_AplCoreMetrics->getViewhostWidth());
        const int pixelHeight = (int) m_AplCoreMetrics->toCorePixel(m_AplCoreMetrics->getViewhostHeight());
        configChange = configChange.size(pixelWidth, pixelHeight);
        sendViewhostScalingMessage();
    }
    // config change for theme
    if (configurationChange.HasMember(DOCTHEME_KEY) && configurationChange[DOCTHEME_KEY].IsString()) {
        configChange = configChange.theme(configurationChange[DOCTHEME_KEY].GetString());
        sendDocumentThemeMessage();
    }
    // config change for mode
    if (configurationChange.HasMember(MODE_KEY) &&
            configurationChange[MODE_KEY].IsString() &&
        AVS_VIEWPORT_MODE_MAP.find(configurationChange[MODE_KEY].GetString()) != AVS_VIEWPORT_MODE_MAP.end()) {
        configChange = configChange.mode(AVS_VIEWPORT_MODE_MAP.at(configurationChange[MODE_KEY].GetString()));
    }
    // config change for fontScale
    if (configurationChange.HasMember(FONTSCALE_KEY) && configurationChange[FONTSCALE_KEY].IsFloat()) {
        configChange = configChange.fontScale(configurationChange[FONTSCALE_KEY].GetFloat());
    }
    // config change for screenMode
    if (configurationChange.HasMember(SCREENMODE_KEY) &&
            configurationChange[SCREENMODE_KEY].IsString() &&
        AVS_SCREEN_MODE_MAP.find(configurationChange[SCREENMODE_KEY].GetString()) != AVS_SCREEN_MODE_MAP.end()) {
        configChange = configChange.screenMode(AVS_SCREEN_MODE_MAP.at(configurationChange[SCREENMODE_KEY].GetString()));
    }
    // config change for screenReader
    if (configurationChange.HasMember(SCREENREADER_KEY) && configurationChange[SCREENREADER_KEY].IsBool()) {
        configChange = configChange.screenReader(configurationChange[SCREENREADER_KEY].GetBool());
    }
    updateConfigurationChange(configChange);
    m_Root->configurationChange(configChange);
}

void AplCoreConnectionManager::executeCommands(const std::string& command, const std::string& token) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Root context is missing");
        return;
    }

    std::shared_ptr<rapidjson::Document> document(new rapidjson::Document);
    if (document->Parse(command).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Parse commands failed");
        return;
    }

    auto it = document->FindMember("commands");
    if (it == document->MemberEnd() || it->value.GetType() != rapidjson::Type::kArrayType) {
        aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Missing commands, or is not array");
        return;
    }

    apl::Object object{it->value};
    auto action = m_Root->executeCommands(object, false);
    if (!action) {
        aplOptions->logMessage(LogLevel::ERROR, "executeCommandsFailed", "Execute commands failed");
        return;
    }

    aplOptions->onActivityStarted(token, APL_COMMAND_EXECUTION);

    action->then([this, document, token](const apl::ActionPtr& action) {
        auto aplOptions = m_aplConfiguration->getAplOptions();
        aplOptions->logMessage(LogLevel::DBG, "executeCommands", "Command sequence complete");
        aplOptions->onCommandExecutionComplete(token, true);
        aplOptions->onActivityEnded(token, APL_COMMAND_EXECUTION);
    });

    action->addTerminateCallback([this, document, token](const apl::TimersPtr&) {
        auto aplOptions = m_aplConfiguration->getAplOptions();
        aplOptions->logMessage(LogLevel::DBG, "executeCommandsFailed", "Command sequence failed");
        aplOptions->onCommandExecutionComplete(token, false);
        aplOptions->onActivityEnded(token, APL_COMMAND_EXECUTION);
    });
}

void AplCoreConnectionManager::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const std::string& source,
    const std::string& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    rapidjson::Document sourceDoc;
    rapidjson::Document paramsDoc;

    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (sourceDoc.Parse(source).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "onExtensionEventFailed", "Parse source failed");
        return;
    }

    if (paramsDoc.Parse(params).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "onExtensionEventFailed", "Parse params failed");
        return;
    }

    m_extensionManager->onExtensionEvent(
        uri, name, apl::Object(sourceDoc), apl::Object(paramsDoc), event, resultCallback);
}

void AplCoreConnectionManager::onExtensionEventResult(unsigned int event, bool succeeded) {
    rapidjson::Document response(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = response.GetAllocator();
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember(EVENT_KEY, event, allocator);
    payload.AddMember(ARGUMENT_KEY, succeeded ? 0 : 1, allocator);
    handleEventResponse(payload);
}

AplDocumentStatePtr AplCoreConnectionManager::getActiveDocumentState() {
    // If we have active content, report it as an AplDocumentState
    if (m_Content && m_Root && m_AplCoreMetrics) {
        auto documentState = std::make_shared<AplDocumentState>(m_aplToken, m_Root, m_AplCoreMetrics);
        return documentState;
    }
    return nullptr;
}

void AplCoreConnectionManager::restoreDocumentState(AplDocumentStatePtr documentState) {
    m_documentStateToRestore = std::move(documentState);
    m_documentStateToRestore->configurationChange = m_ConfigurationChange;
    reset();
    m_aplConfiguration->getAplOptions()->resetViewhost(m_documentStateToRestore->token);
}

void AplCoreConnectionManager::invokeExtensionEventHandler(
        const std::string& uri,
        const std::string& name,
        const apl::ObjectMap& data,
        bool fastMode) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "invokeExtensionEventHandlerFailed", "Root context is missing");
        return;
    }
    aplOptions->logMessage(LogLevel::DBG, "invokeExtensionEventHandler", +"< " + uri + ":" + name + " >");
    m_Root->invokeExtensionEventHandler(uri, name, data, fastMode);
}

void AplCoreConnectionManager::dataSourceUpdate(
        const std::string& sourceType,
        const std::string& jsonPayload,
        const std::string& token) {

    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Root context is missing");
        return;
    }

    auto provider = m_Root->getRootConfig().getDataSourceProvider(sourceType);
    if (!provider) {
        aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Unknown provider requested.");
        return;
    }

    bool result = provider->processUpdate(jsonPayload);
    if (!result) {
        aplOptions->logMessage(LogLevel::ERROR, "dataSourceUpdateFailed", "Update is not processed.");
        checkAndSendDataSourceErrors();
    }
}

void AplCoreConnectionManager::provideState(unsigned int stateRequestToken) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    auto timer = m_aplConfiguration->getMetricsRecorder()->createTimer(
            Telemetry::AplMetricsRecorderInterface::CURRENT_DOCUMENT,
            "APL-Web.RootContext.notifyVisualContext");
    timer->start();
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
        aplOptions->logMessage(LogLevel::ERROR, "provideStateFailed", "Unable to get visual context");
        rapidjson::Value emptyObj(rapidjson::kObjectType);
        // add an empty visual context
        arr.PushBack(emptyObj, allocator);
    }
    // Add visual context info
    state.AddMember(CONTEXT_KEY, arr, allocator);
    state.Accept(writer);
    aplOptions->onVisualContextAvailable(m_aplToken, stateRequestToken, buffer.GetString());
    timer->stop();
}

void AplCoreConnectionManager::interruptCommandSequence() {
    if (m_Root) {
        m_Root->cancelExecution();
    }
}

void AplCoreConnectionManager::handleBuild(const rapidjson::Value& message) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    auto inflationTimer = m_aplConfiguration->getMetricsRecorder()->createTimer(
            Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
            Telemetry::AplRenderingSegment::kRootContextInflation);
    inflationTimer->start();

    /* APL Document Inflation started */
    aplOptions->onRenderingEvent(m_aplToken, AplRenderingEvent::INFLATE_BEGIN);


    apl::RootConfig config;

    if (m_documentStateToRestore) {
        // Restore from document state
        m_aplToken = m_documentStateToRestore->token;
        m_Root = m_documentStateToRestore->rootContext;
        m_Content = m_Root->content();
        config = m_documentStateToRestore->rootContext->getRootConfig();
        m_Root->configurationChange(m_documentStateToRestore->configurationChange);
        coreFrameUpdate();
    }

    if (!m_Content) {
        aplOptions->logMessage(LogLevel::WARN, "handleBuildFailed", "No content to build");
        sendError("No content to build");
        inflationTimer->fail();
        return;
    }

    // Get APL Version for content
    std::string aplVersion = m_Content->getAPLVersion();

    // If we're not restoring a document state, create a new RootConfig.
    if (!m_documentStateToRestore) {
        std::string agentName = getOptionalValue(message, AGENTNAME_KEY, "wssHost");
        std::string agentVersion = getOptionalValue(message, AGENTVERSION_KEY, "1.0");
        bool allowOpenUrl = getOptionalBool(message, ALLOWOPENURL_KEY, false);
        bool disallowVideo = getOptionalBool(message, DISALLOWVIDEO_KEY, false);
        int animationQuality =
            getOptionalInt(message, ANIMATIONQUALITY_KEY, apl::RootConfig::AnimationQuality::kAnimationQualityNormal);

        config = apl::RootConfig()
                     .agent(agentName, agentVersion)
                     .allowOpenUrl(allowOpenUrl)
                     .disallowVideo(disallowVideo)
                     .animationQuality(static_cast<apl::RootConfig::AnimationQuality>(animationQuality))
                     .measure(std::make_shared<AplCoreTextMeasurement>(shared_from_this(), m_aplConfiguration))
                     .localeMethods(std::make_shared<AplCoreLocaleMethods>(shared_from_this(), m_aplConfiguration))
                     .utcTime(getCurrentTime().count())
                     .localTimeAdjustment(aplOptions->getTimezoneOffset().count())
                     .enforceAPLVersion(apl::APLVersion::kAPLVersionIgnore)
                     .sequenceChildCache(5)
                     .enableExperimentalFeature(apl::RootConfig::ExperimentalFeature::kExperimentalFeatureHandleScrollingAndPagingInCore)
                     .enableExperimentalFeature(apl::RootConfig::ExperimentalFeature::kExperimentalFeatureNotifyChildrenChangedOnDisplayChange)
                     .enableExperimentalFeature(apl::RootConfig::ExperimentalFeature::kExperimentalFeatureHandleFocusInCore)
                     .set(apl::RootProperty::kDefaultIdleTimeout, -1);

        // Data Sources
        config.dataSourceProvider(
            apl::DynamicIndexListConstants::DEFAULT_TYPE_NAME,
            std::make_shared<apl::DynamicIndexListDataSourceProvider>());

        config.dataSourceProvider(
                apl::DynamicTokenListConstants::DEFAULT_TYPE_NAME,
                std::make_shared<apl::DynamicTokenListDataSourceProvider>());
    }

    // Add Extensions which are supported, requested, and available to the config
    if (message.HasMember(SUPPORTED_EXTENSIONS) && message[SUPPORTED_EXTENSIONS].IsArray()) {
        // Extensions supported by the client renderer instance
        auto supportedExtensions = message[SUPPORTED_EXTENSIONS].GetArray();
        // Extensions requested by the content
        auto requestedExtensions = m_Content->getExtensionRequests();
        for (auto& ext : supportedExtensions) {
            auto uri = ext.GetString();
            // If the supported extension is both requested and available, register it with the config
            if (requestedExtensions.find(uri) != requestedExtensions.end()) {
                if (auto extension = m_extensionManager->getExtension(uri)) {
                    // Apply content defined settings to extension
                    extension->applySettings(m_Content->getExtensionSettings(uri));
                    m_extensionManager->registerRequestedExtension(extension->getUri(), config);
                }
            }
        }
    }

    auto renderingOptionsMsg = AplCoreViewhostMessage(RENDERING_OPTIONS_KEY);
    rapidjson::Value renderingOptions(rapidjson::kObjectType);
    renderingOptions.AddMember(LEGACY_KARAOKE_KEY, aplVersion == "1.0", renderingOptionsMsg.alloc());
    send(renderingOptionsMsg.setPayload(std::move(renderingOptions)));

    m_PendingEvents.clear();

    // Release the activity tracker
    aplOptions->onActivityEnded(m_aplToken, APL_COMMAND_EXECUTION);

    if (m_ScreenLock) {
        aplOptions->onActivityEnded(m_aplToken, APL_SCREEN_LOCK);
        m_ScreenLock = false;
    }

    m_StartTime = getCurrentTime();

    // If we're not restoring a document state, then create metrics and RootContext
    if (!m_documentStateToRestore) {
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

            sendViewhostScalingMessage();

            m_renderingStart = std::chrono::steady_clock::now();
            m_StartTime = getCurrentTime();
            m_Root = apl::RootContext::create(m_AplCoreMetrics->getMetrics(), m_Content, config);
            if (m_Root) {
                break;
            } else if (!m_ViewportSizeSpecifications.empty()) {
                aplOptions->logMessage(
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
    }

    // Make sure we only restore a documentState once.
    if (m_documentStateToRestore) {
        m_documentStateToRestore.reset();
    }

    // Get background
    apl::Object background = m_Content->getBackground(m_AplCoreMetrics->getMetrics(), config);

    bool supportsResizing = false;

    // Get Document Settings
    if (auto documentSettings = m_Content->getDocumentSettings()) {
        // Get resizing setting
        supportsResizing = documentSettings->getValue(SUPPORTS_RESIZING_KEY).asBoolean();
    }

    sendSupportsResizingMessage(supportsResizing);

    /* APL Core Inflation ended */
    aplOptions->onRenderingEvent(m_aplToken, AplRenderingEvent::INFLATE_END);

    if (m_Root) {
        inflationTimer->stop();
        // Init viewhost globals
        sendViewhostScalingMessage();
        sendDocumentThemeMessage();
        sendDocumentBackgroundMessage(background);

        // Start rendering component hierarchy
        auto reply = AplCoreViewhostMessage(HIERARCHY_KEY);
        send(reply.setPayload(m_Root->topComponent()->serialize(reply.alloc())));

        auto idleTimeout = std::chrono::milliseconds(m_Root->settings().idleTimeout(config));
        aplOptions->onSetDocumentIdleTimeout(m_aplToken, idleTimeout);
        aplOptions->onRenderDocumentComplete(m_aplToken, true, "");
    } else {
        inflationTimer->fail();
        aplOptions->logMessage(LogLevel::ERROR, "handleBuildFailed", "Unable to inflate document");
        sendError("Unable to inflate document");
        aplOptions->onRenderDocumentComplete(m_aplToken, false, "Unable to inflate document");
        // Send DataSource errors if any
        checkAndSendDataSourceErrors();
    }
}

void AplCoreConnectionManager::onDocumentRendered(
        const std::chrono::steady_clock::time_point &renderTime,
        uint64_t complexityScore) {
    if (m_renderingStart.time_since_epoch() > std::chrono::milliseconds(0)) {
        auto metricsRecorder = m_aplConfiguration->getMetricsRecorder();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(renderTime - m_renderingStart);

        auto timer = metricsRecorder->createTimer(
            Telemetry::AplMetricsRecorderInterface::CURRENT_DOCUMENT,
            "APL-Web.renderDocument");
        timer->elapsed(elapsed);

        if (complexityScore > 0) {
            auto complexityTimer = metricsRecorder->createTimer(
                Telemetry::AplMetricsRecorderInterface::CURRENT_DOCUMENT,
                "APL-Web.renderDocument." + std::to_string(complexityScore));
            complexityTimer->elapsed(elapsed);
        }

        metricsRecorder->flush();
        m_renderingStart = std::chrono::steady_clock::time_point(std::chrono::milliseconds(0));
    }

}

void AplCoreConnectionManager::sendViewhostScalingMessage() {
    if (m_AplCoreMetrics) {
        // Send scaling metrics out to viewhost
        auto reply = AplCoreViewhostMessage(SCALING_KEY);
        rapidjson::Value scaling(rapidjson::kObjectType);
        scaling.AddMember(SCALE_FACTOR_KEY, m_AplCoreMetrics->toViewhost(1.0f), reply.alloc());
        scaling.AddMember(VIEWPORT_WIDTH_KEY, m_AplCoreMetrics->getViewhostWidth(), reply.alloc());
        scaling.AddMember(VIEWPORT_HEIGHT_KEY, m_AplCoreMetrics->getViewhostHeight(), reply.alloc());
        send(reply.setPayload(std::move(scaling)));
    }
}

void AplCoreConnectionManager::sendDocumentThemeMessage() {
    if (m_Root) {
        auto themeMsg = AplCoreViewhostMessage(DOCTHEME_KEY);
        auto& alloc = themeMsg.alloc();
        rapidjson::Value payload(rapidjson::kObjectType);
        std::string docTheme = "dark";
        docTheme = m_Root->getTheme();
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

void AplCoreConnectionManager::sendSupportsResizingMessage(bool supportsResizing) {
    auto supportsResizingMsg = AplCoreViewhostMessage(SUPPORTS_RESIZING_KEY);
    auto& alloc = supportsResizingMsg.alloc();
    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember(SUPPORTS_RESIZING_KEY, supportsResizing, alloc);
    supportsResizingMsg.setPayload(std::move(payload));
    send(supportsResizingMsg);
}

void AplCoreConnectionManager::handleUpdate(const rapidjson::Value& update) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->findComponentById(id);
    if (!component) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleUpdateFailed", std::string("Unable to find component with id: ") + id);
        sendError("Unable to find component");
        return;
    }

    auto type = static_cast<apl::UpdateType>(update["type"].GetInt());

    if (update["value"].IsString()) {
        std::string value = update["value"].GetString();
        component->update(type, value);
    } else {
        auto value = update["value"].GetFloat();

        if (type == apl::UpdateType::kUpdateScrollPosition) {
            value = m_AplCoreMetrics->toCore(value);
        }

        component->update(type, value);
    }
}

void AplCoreConnectionManager::handleMediaUpdate(const rapidjson::Value& update) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleMediaUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->findComponentById(id);
    if (!component) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleMediaUpdateFailed", std::string("Unable to find component with id: ") + id);
        sendError("Unable to find component");
        return;
    }

    if (!update.HasMember(MEDIA_STATE_KEY) || !update.HasMember(FROM_EVENT_KEY)) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleMediaUpdateFailed", "State update object is missing parameters");
        sendError("Can't update media state.");
        return;
    }
    auto& state = update[MEDIA_STATE_KEY];
    auto fromEvent = update[FROM_EVENT_KEY].GetBool();

    if (!state.HasMember(TRACK_INDEX_KEY) || !state.HasMember(TRACK_COUNT_KEY) || !state.HasMember(CURRENT_TIME_KEY) ||
        !state.HasMember(DURATION_KEY) || !state.HasMember(PAUSED_KEY) || !state.HasMember(ENDED_KEY)) {
        aplOptions->logMessage(
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
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleGraphicUpdateFailed", "Root context is null");
        return;
    }

    auto id = update["id"].GetString();
    auto component = m_Root->findComponentById(id);
    if (!component) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleGraphicUpdateFailed", std::string("Unable to find component with id:") + id);
        sendError("Unable to find component");
        return;
    }

    auto json = apl::GraphicContent::create(update["avg"].GetString());
    component->updateGraphic(json);
}

void AplCoreConnectionManager::handleEnsureLayout(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleEnsureLayoutFailed", "Root context is null");
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->findComponentById(id);
    if (!component) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleEnsureLayoutFailed", std::string("Unable to find component with id:") + id);
        sendError("Unable to find component");
        return;
    }

    component->ensureLayout(true);
    auto msg = AplCoreViewhostMessage(ENSURELAYOUT_KEY);
    send(msg.setPayload(id));
}

void AplCoreConnectionManager::handleScrollToRectInComponent(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleScrollToRectInComponentFailed", "Root context is null");
        return;
    }

    auto id = payload["id"].GetString();
    auto component = m_Root->findComponentById(id);
    if (!component) {
        aplOptions->logMessage(
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
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleHandleKeyboardFailed", "Root context is null");
        return;
    }

    auto messageId = payload["messageId"].GetString();
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
    bool result = m_Root->handleKeyboard(static_cast<apl::KeyHandlerType>(keyType), keyboard);

    auto handleKeyboardResultMessage = AplCoreViewhostMessage("handleKeyboard");
    auto& alloc = handleKeyboardResultMessage.alloc();
    rapidjson::Value handleKeyboardResultValue(rapidjson::kObjectType);

    rapidjson::Value value;
    value.SetString(messageId, alloc);
    handleKeyboardResultValue.AddMember("messageId", value, alloc);
    handleKeyboardResultValue.AddMember("result", result, alloc);

    handleKeyboardResultMessage.setPayload(std::move(handleKeyboardResultValue));
    send(handleKeyboardResultMessage);
}

void AplCoreConnectionManager::getFocusableAreas(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "getFocusableAreasFailed", "Root context is null");
        return;
    }

    auto messageId = payload["messageId"].GetString();
    auto message = AplCoreViewhostMessage("getFocusableAreas");
    auto result = m_Root->getFocusableAreas();
    auto& alloc = message.alloc();

    rapidjson::Value outPayload(rapidjson::kObjectType);
    rapidjson::Value value;
    value.SetString(messageId, alloc);
    outPayload.AddMember("messageId", value, alloc);

    rapidjson::Value areas(rapidjson::kObjectType);

    for (auto iterator = result.begin(); iterator != result.end(); iterator++) {
        auto top = iterator->second.getTop();
        auto left = iterator->second.getLeft();
        auto width = iterator->second.getWidth();
        auto height = iterator->second.getHeight();

        rapidjson::Value rectangle(rapidjson::kObjectType);
        rectangle.AddMember("top", top, alloc);
        rectangle.AddMember("left", left, alloc);
        rectangle.AddMember("width", width, alloc);
        rectangle.AddMember("height", height, alloc);
        rapidjson::Value key(rapidjson::kStringType);
        key.SetString(iterator->first.c_str(), alloc);
        areas.AddMember(key, rectangle, alloc);
    }

    outPayload.AddMember("areas", areas, alloc);
    message.setPayload(std::move(outPayload));
    send(message);
}

void AplCoreConnectionManager::getFocused(const rapidjson::Value& payload) {
    auto messageId = payload["messageId"].GetString();
    auto message = AplCoreViewhostMessage("getFocused");

    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "getFocusedFailed", "Root context is null");
        return;
    }

    auto result = m_Root->getFocused();
    auto& alloc = message.alloc();
    rapidjson::Value outPayload(rapidjson::kObjectType);
    rapidjson::Value value;
    value.SetString(messageId, alloc);
    outPayload.AddMember("messageId", value, alloc);
    outPayload.AddMember("result", result, alloc);
    message.setPayload(std::move(outPayload));
    send(message);
}

void AplCoreConnectionManager::setFocus(const rapidjson::Value& payload) {
    auto direction = payload["direction"].GetInt();
    auto top = payload["origin"].FindMember("top")->value.Get<float>();
    auto left = payload["origin"].FindMember("left")->value.Get<float>();
    auto width = payload["origin"].FindMember("width")->value.Get<float>();
    auto height = payload["origin"].FindMember("height")->value.Get<float>();

    auto origin = apl::Rect(top,left,width,height);
    auto targetId = payload["targetId"].GetString();
    m_Root->setFocus(static_cast<apl::FocusDirection>(direction), origin, targetId);
}

void AplCoreConnectionManager::handleUpdateCursorPosition(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleUpdateCursorPositionFailed", "Root context is null");
        return;
    }

    const float x = payload[X_KEY].GetFloat();
    const float y = payload[Y_KEY].GetFloat();
    apl::Point cursorPosition(m_AplCoreMetrics->toCore(x), m_AplCoreMetrics->toCore(y));
    m_Root->updateCursorPosition(cursorPosition);
}

void AplCoreConnectionManager::handleHandlePointerEvent(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleHandlePointerEventFailed", "Root context is null");
        return;
    }

    const float x = payload[X_KEY].GetFloat();
    const float y = payload[Y_KEY].GetFloat();

    apl::Point point = apl::Point(m_AplCoreMetrics->toCore(x), m_AplCoreMetrics->toCore(y));
    apl::PointerEventType pointerEventType = static_cast<apl::PointerEventType>(payload[POINTEREVENTTYPE_KEY].GetInt());
    apl::PointerType pointerType = static_cast<apl::PointerType>(payload[POINTERTYPE_KEY].GetInt());
    auto pointerId = static_cast<apl::id_type>(payload[POINTERID_KEY].GetInt());
    apl::PointerEvent pointerEvent = apl::PointerEvent(pointerEventType, point, pointerId, pointerType);

    m_Root->handlePointerEvent(pointerEvent);
}

void AplCoreConnectionManager::handleEventResponse(const rapidjson::Value& response) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleEventResponseFailed", "Root context is null");
        return;
    }

    if (!response[EVENT_KEY].IsInt()) {
        aplOptions->logMessage(LogLevel::ERROR, "handleEventResponseFailed", "Invalid event response");
        sendError("Invalid event response");
        return;
    }
    auto event = response[EVENT_KEY].GetInt();
    auto it = m_PendingEvents.find(event);
    if (it != m_PendingEvents.end()) {
        auto rectJson = response.FindMember("rectArgument");
        if (rectJson != response.MemberEnd()) {
            apl::Rect rect = convertJsonToScaledRect(rectJson->value);
            it->second.resolve(rect);
        } else {
            auto arg = response.FindMember(ARGUMENT_KEY);
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
    m_aplConfiguration->getAplOptions()->sendMessage(m_aplToken, message.setSequenceNumber(seqno).get());
    return seqno;
}

rapidjson::Document AplCoreConnectionManager::blockingSend(
    AplCoreViewhostMessage& message,
    const std::chrono::milliseconds& timeout) {
    std::lock_guard<std::mutex> lock{m_blockingSendMutex};
    m_replyPromise = std::promise<std::string>();
    m_blockingSendReplyExpected = true;
    m_replyExpectedSequenceNumber = send(message);

    auto aplOptions = m_aplConfiguration->getAplOptions();
    auto future = m_replyPromise.get_future();
    auto status = future.wait_for(timeout);
    if (status != std::future_status::ready) {
        m_blockingSendReplyExpected = false;
        // Under the situation that finish command destroys the renderer, there is no response.
        aplOptions->logMessage(LogLevel::WARN, "blockingSendFailed", "Did not receive response");
        return rapidjson::Document(rapidjson::kNullType);
    }

    rapidjson::Document doc;
    if (doc.Parse(future.get()).HasParseError()) {
        aplOptions->logMessage(LogLevel::ERROR, "blockingSendFailed", "parsingFailed");
        return rapidjson::Document(rapidjson::kNullType);
    }

    return doc;
}

void AplCoreConnectionManager::sendError(const std::string& message) {
    auto reply = AplCoreViewhostMessage(ERROR_KEY);
    send(reply.setPayload(message));
}

void AplCoreConnectionManager::handleScreenLock() {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (m_Root->screenLock() && !m_ScreenLock) {
        aplOptions->onActivityStarted(m_aplToken, APL_SCREEN_LOCK);
        m_ScreenLock = true;
    } else if (!m_Root->screenLock() && m_ScreenLock) {
        aplOptions->onActivityEnded(m_aplToken, APL_SCREEN_LOCK);
        m_ScreenLock = false;
    } else {
        return;
    }
    sendScreenLockMessage(m_ScreenLock);
}

void AplCoreConnectionManager::processEvent(const apl::Event& event) {
    auto aplOptions = m_aplConfiguration->getAplOptions();

    if (apl::EventType::kEventTypeFinish == event.getType()) {
        aplOptions->onFinish(m_aplToken);
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

        aplOptions->onSendEvent(m_aplToken, sb.GetString());
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

        aplOptions->onDataSourceFetchRequestEvent(m_aplToken, type.asString(), sb.GetString());
        return;
    }

    if (apl::EventType::kEventTypeExtension == event.getType()) {
        /**
         * Extension Events are received when registered ExtensionCommands are fired
         * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioncommanddefinition.h
         */
        rapidjson::Document extensionEventPayloadJson(rapidjson::kObjectType);
        auto& allocator = extensionEventPayloadJson.GetAllocator();

        auto uri = event.getValue(apl::EventProperty::kEventPropertyExtensionURI);
        auto name = event.getValue(apl::EventProperty::kEventPropertyName);
        auto source = event.getValue(apl::EventProperty::kEventPropertySource);
        auto params = event.getValue(apl::EventProperty::kEventPropertyExtension);

        std::string sourceStr;
        std::string paramsStr;

        serializeJSONValueToString(source.serialize(allocator).Move(), &sourceStr);
        serializeJSONValueToString(params.serialize(allocator).Move(), &paramsStr);

        /**
         * If the registered ExtensionCommand requires resolution, the resultCallback should be registered with the
         * extension
         * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioncommanddefinition.h#L87
         */
        auto token = ++m_SequenceNumber;
        auto resultCallback = addPendingEvent(token, event, false) ? shared_from_this() : nullptr;
        aplOptions->onExtensionEvent(m_aplToken, uri.getString(), name.getString(), sourceStr, paramsStr, token, resultCallback);
        return;
    }

    auto msg = AplCoreViewhostMessage(EVENT_KEY);
    auto token = send(msg.setPayload(event.serialize(msg.alloc())));
    addPendingEvent(token, event);
}

bool AplCoreConnectionManager::addPendingEvent(unsigned int token, const apl::Event& event, bool isViewhostEvent) {
    // If the event had an action ref, stash the reference for future use
    auto ref = event.getActionRef();
    if (!ref.isEmpty()) {
        m_PendingEvents.emplace(token, ref);
        ref.addTerminateCallback([this, token, isViewhostEvent](const apl::TimersPtr&) {
            auto it = m_PendingEvents.find(token);
            if (it != m_PendingEvents.end()) {
                if (isViewhostEvent) {
                    auto msg = AplCoreViewhostMessage(EVENT_TERMINATE_KEY);
                    rapidjson::Value payload(rapidjson::kObjectType);
                    payload.AddMember("token", token, msg.alloc());
                    send(msg.setPayload(std::move(payload)));
                }

                m_PendingEvents.erase(it);  // Remove the pending event
            } else {
                m_aplConfiguration->getAplOptions()->logMessage(LogLevel::WARN, __func__, "Event was not pending");
            }
        });
        return true;
    }
    return false;
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
        if (component->getDirty().count(apl::kPropertyGraphic)) {
            // for graphic component, apl-client need walk into graphicPtr to get dirty and dirtyPropertyKeys.
            rapidjson::Value vectorGraphicComponent = component->serializeDirty(msg.alloc());
            rapidjson::Value dirtyGraphicElement(rapidjson::kArrayType);
            const apl::GraphicPtr graphic = component->getCalculated(apl::kPropertyGraphic).getGraphic();
            for (auto& graphicDirty : graphic->getDirty()) {
                rapidjson::Value serializedGraphicElement = graphicDirty->serialize(msg.alloc());
                rapidjson::Value dirtyPropertyKeys(rapidjson::kArrayType);
                for (auto& dirtyPropertyKey : graphicDirty->getDirtyProperties()) {
                    dirtyPropertyKeys.PushBack(dirtyPropertyKey, msg.alloc());
                }
                serializedGraphicElement.AddMember("dirtyProperties", dirtyPropertyKeys, msg.alloc());
                dirtyGraphicElement.PushBack(serializedGraphicElement, msg.alloc());
            }
            if(vectorGraphicComponent.HasMember("graphic") && vectorGraphicComponent["graphic"].IsObject()) {
                vectorGraphicComponent["graphic"].AddMember("dirty", dirtyGraphicElement, msg.alloc());
            }
            tempDirty[component->getUniqueId()] = vectorGraphicComponent;
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
    auto aplOptions = m_aplConfiguration->getAplOptions();
    auto now = getCurrentTime() - m_StartTime;
    m_Root->updateTime(now.count(), getCurrentTime().count());
    m_Root->setLocalTimeAdjustment(aplOptions->getTimezoneOffset().count());

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
    if (!m_Root) return;

    std::vector<apl::Object> errorArray;

    for (auto& type : KNOWN_DATA_SOURCES) {
        auto provider = m_Root->getRootConfig().getDataSourceProvider(type);

        if (provider) {
            auto pendingErrors = provider->getPendingErrors();
            if (!pendingErrors.empty() && pendingErrors.isArray()) {
                errorArray.insert(errorArray.end(), pendingErrors.getArray().begin(), pendingErrors.getArray().end());
            }
        }
    }

    auto errors = apl::Object(std::make_shared<apl::ObjectArray>(errorArray));

    if (!errors.empty()) {
        auto errorEvent = std::make_shared<apl::ObjectMap>();
        errorEvent->emplace(PRESENTATION_TOKEN_KEY, m_aplToken);
        errorEvent->emplace(ERRORS_KEY, errors);

        rapidjson::Document runtimeErrorPayloadJson(rapidjson::kObjectType);
        auto& allocator = runtimeErrorPayloadJson.GetAllocator();
        auto runtimeError = apl::Object(errorEvent).serialize(allocator);

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        runtimeError.Accept(writer);

        m_aplConfiguration->getAplOptions()->onRuntimeErrorEvent(m_aplToken, sb.GetString());
    }
}

const std::string AplCoreConnectionManager::getAPLToken() {
    return m_aplToken;
}

void AplCoreConnectionManager::serializeJSONValueToString(const rapidjson::Value& documentNode, std::string* value) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);

    if (!documentNode.Accept(writer)) {
        auto aplOptions = m_aplConfiguration->getAplOptions();
        aplOptions->logMessage(LogLevel::ERROR, "serializeJSONValueToStringFailed", "acceptFailed");
        return;
    }

    *value = stringBuffer.GetString();
}

void AplCoreConnectionManager::addExtensions(
    std::unordered_set<std::shared_ptr<AplCoreExtensionInterface>> extensions) {
    for (auto& extension : extensions) {
        extension->setEventHandler(shared_from_this());
        m_extensionManager->addExtension(extension);
    }
}

std::shared_ptr<AplCoreExtensionInterface> AplCoreConnectionManager::getExtension(const std::string& uri) {
    return m_extensionManager->getExtension(uri);
}

void AplCoreConnectionManager::reset() {
    m_aplToken = "";
    m_Root.reset();
    m_Content.reset();
}

void AplCoreConnectionManager::handleIsCharacterValid(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleIsCharacterValidFailed", "Root context is null");
        return;
    }

    auto messageId = payload["messageId"].GetString();
    if (!messageId) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleIsCharacterValidFailed", std::string("Payload does not contain messageId"));
        sendError("Payload does not contain messageId");
        return;
    }

    auto character = payload["character"].GetString();
    if (!character) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleIsCharacterValidFailed", std::string("Payload does not contain character"));
        sendError("Payload does not contain character");
        return;
    }

    auto componentId = payload["componentId"].GetString();
    if (!componentId) {
        aplOptions->logMessage(
            LogLevel::ERROR, "handleIsCharacterValidFailed", std::string("Payload does not contain componentId"));
        sendError("Payload does not contain componentId");
        return;
    }
    auto component = m_Root->findComponentById(componentId);
    if (!component) {
        aplOptions->logMessage(
            LogLevel::ERROR,
            "handleIsCharacterValidFailed",
            std::string("Unable to find component with id: ") + componentId);
        sendError("Unable to find component");
        return;
    }

    auto result = component->isCharacterValid(character[0]);

    auto resultMessage = AplCoreViewhostMessage("isCharacterValid");
    auto& alloc = resultMessage.alloc();
    rapidjson::Value resultMessageValue(rapidjson::kObjectType);

    rapidjson::Value messageIdValue;
    messageIdValue.SetString(messageId, alloc);
    resultMessageValue.AddMember("messageId", messageIdValue, alloc);

    resultMessageValue.AddMember("valid", result, alloc);

    rapidjson::Value componentIdValue;
    componentIdValue.SetString(componentId, alloc);
    resultMessageValue.AddMember("componentId", componentIdValue, alloc);

    resultMessage.setPayload(std::move(resultMessageValue));
    send(resultMessage);
}

void AplCoreConnectionManager::handleReInflate(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleIsCharacterValidFailed", "Root context is null");
        return;
    }
    m_Root->reinflate();

    // update component hierarchy
    auto reply = AplCoreViewhostMessage(HIERARCHY_KEY);
    send(reply.setPayload(m_Root->topComponent()->serialize(reply.alloc())));
}

void AplCoreConnectionManager::handleReHierarchy(const rapidjson::Value& payload) {
    // send component hierarchy
    auto reply = AplCoreViewhostMessage(REHIERARCHY_KEY);
    blockingSend(reply.setPayload(m_Root->topComponent()->serialize(reply.alloc())));
}

void AplCoreConnectionManager::updateConfigurationChange(const apl::ConfigurationChange& configurationChange) {
    m_ConfigurationChange.mergeConfigurationChange(configurationChange);
}

void AplCoreConnectionManager::handleGetDisplayedChildCount(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleGetDisplayedChildCountFailed", "Root context is null");
        return;
    }

    auto messageId = payload["messageId"].GetString();
    if (!messageId) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildCountFailed", std::string("Payload does not contain messageId"));
        sendError("Payload does not contain messageId");
        return;
    }

    auto componentId = payload["componentId"].GetString();
    if (!componentId) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildCountFailed", std::string("Payload does not contain componentId"));
        sendError("Payload does not contain componentId");
        return;
    }
    auto component = m_Root->findComponentById(componentId);
    if (!component) {
        aplOptions->logMessage(
                LogLevel::ERROR,
                "handleGetDisplayedChildCountFailed",
                std::string("Unable to find component with id: ") + componentId);
        sendError("Unable to find component");
        return;
    }

    auto result = component->getDisplayedChildCount();

    auto resultMessage = AplCoreViewhostMessage("getDisplayedChildCount");
    auto& alloc = resultMessage.alloc();
    rapidjson::Value resultMessageValue(rapidjson::kObjectType);

    rapidjson::Value messageIdValue;
    messageIdValue.SetString(messageId, alloc);
    resultMessageValue.AddMember("messageId", messageIdValue, alloc);

    resultMessageValue.AddMember("displayedChildCount", static_cast<unsigned int>(result), alloc);

    rapidjson::Value componentIdValue;
    componentIdValue.SetString(componentId, alloc);
    resultMessageValue.AddMember("componentId", componentIdValue, alloc);

    resultMessage.setPayload(std::move(resultMessageValue));
    send(resultMessage);
}

void AplCoreConnectionManager::handleGetDisplayedChildId(const rapidjson::Value& payload) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    if (!m_Root) {
        aplOptions->logMessage(LogLevel::ERROR, "handleGetDisplayedChildIdFailed", "Root context is null");
        return;
    }

    auto messageId = payload["messageId"].GetString();
    if (!messageId) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildIdFailed", std::string("Payload does not contain messageId"));
        sendError("Payload does not contain messageId");
        return;
    }

    auto componentId = payload["componentId"].GetString();
    if (!componentId) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildIdFailed", std::string("Payload does not contain componentId"));
        sendError("Payload does not contain componentId");
        return;
    }

    auto displayIndexString = payload["displayIndex"].GetString();
    if (!displayIndexString) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildIdFailed", std::string("Payload does not contain displayIndex"));
        sendError("Payload does not contain displayIndex");
        return;
    }

    auto component = m_Root->findComponentById(componentId);
    if (!component) {
        aplOptions->logMessage(
                LogLevel::ERROR,
                "handleGetDisplayedChildIdFailed",
                std::string("Unable to find component with id: ") + componentId);
        sendError("Unable to find component");
        return;
    }

    auto displayedChildCount = component->getDisplayedChildCount();
    auto displayIndex = std::stoi(displayIndexString);

    if (displayIndex >= displayedChildCount) {
        aplOptions->logMessage(
                LogLevel::ERROR, "handleGetDisplayedChildIdFailed", std::string("Asked for a component out of bounds."));
        sendError("Asked for a component out of bounds.");
        return;
    }
    auto displayedChild = component->getDisplayedChildAt(displayIndex);
    auto displayedChildId = displayedChild->getUniqueId();

    auto resultMessage = AplCoreViewhostMessage("getDisplayedChildId");
    auto& alloc = resultMessage.alloc();
    rapidjson::Value resultMessageValue(rapidjson::kObjectType);

    rapidjson::Value messageIdValue;
    messageIdValue.SetString(messageId, alloc);
    resultMessageValue.AddMember("messageId", messageIdValue, alloc);

    resultMessageValue.AddMember("displayedChildId", displayedChildId, alloc);

    rapidjson::Value componentIdValue;
    componentIdValue.SetString(componentId, alloc);
    resultMessageValue.AddMember("componentId", componentIdValue, alloc);

    resultMessage.setPayload(std::move(resultMessageValue));
    send(resultMessage);
}
}  // namespace APLClient
