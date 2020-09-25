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

#include <AVSCommon/Utils/Timing/Timer.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SampleApp/Messages/GUIClientMessage.h>
#include "SampleApp/AplClientBridge.h"
#include "SampleApp/CachingDownloadManager.h"
#include "SampleApp/DownloadMonitor.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

static const std::string TAG{"AplClientBridge"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

using namespace alexaClientSDK::avsCommon::avs::attachment;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::libcurlUtils;
using namespace alexaClientSDK::avsCommon::utils::sds;
using namespace alexaClientSDK::avsCommon::utils::timing;
using namespace smartScreenSDKInterfaces;
using namespace APLClient::Extensions;

/// Default string to attach to mainTemplate parameters.
static const std::string DEFAULT_PARAM_VALUE = "{}";

std::shared_ptr<AplClientBridge> AplClientBridge::create(
    std::shared_ptr<CachingDownloadManager> contentDownloadManager,
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
    AplClientBridgeParameter parameters) {
    std::shared_ptr<AplClientBridge> aplClientBridge(
        new AplClientBridge(contentDownloadManager, guiClient, parameters));
    aplClientBridge->m_aplClient.reset(new APLClient::AplClientBinding(aplClientBridge));
    aplClientBridge->m_aplClientRenderer = aplClientBridge->m_aplClient->createRenderer("");
    aplClientBridge->loadExtensions();
    return aplClientBridge;
}

AplClientBridge::AplClientBridge(
    std::shared_ptr<CachingDownloadManager> contentDownloadManager,
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
    AplClientBridgeParameter parameters) :
        m_contentDownloadManager{contentDownloadManager},
        m_guiClient{guiClient},
        m_renderQueued{false},
        m_parameters{parameters} {
    m_playerActivityState = alexaClientSDK::avsCommon::avs::PlayerActivity::FINISHED;
}

void AplClientBridge::loadExtensions() {
    ACSDK_DEBUG9(LX(__func__));

    // AudioPlayer Extension
    m_audioPlayerExtension = std::make_shared<AudioPlayer::AplAudioPlayerExtension>(shared_from_this());

    // Backstack Extension
    m_backstackExtension = std::make_shared<Backstack::AplBackstackExtension>(shared_from_this());

    addExtensions({m_audioPlayerExtension, m_backstackExtension});
}

void AplClientBridge::addExtensions(std::unordered_set<std::shared_ptr<AplCoreExtensionInterface>> extensions) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, extensions] { m_aplClientRenderer->addExtensions(extensions); });
}

void AplClientBridge::sendMessage(const std::string& token, const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));

    auto aplCoreMessage = messages::AplCoreMessage(payload);
    m_guiClient->sendMessage(aplCoreMessage);
}

void AplClientBridge::resetViewhost(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    auto message = messages::AplRenderMessage(m_windowId, token);
    m_guiClient->sendMessage(message);
}

std::string AplClientBridge::downloadResource(const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    auto downloadMetricsEmitter = m_aplClient->createDownloadMetricsEmitter();
    auto observer = std::make_shared<DownloadMonitor>(downloadMetricsEmitter);
    return m_contentDownloadManager->retrieveContent(source, observer);
}

std::chrono::milliseconds AplClientBridge::getTimezoneOffset() {
    // Relies on fact that this is always called in the executor thread via either RenderDocument or UpdateTimer
    return m_guiManager->getDeviceTimezoneOffset();
}

void AplClientBridge::onActivityStarted(const std::string& token, const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, source] { m_guiManager->handleActivityEvent(source, ActivityEvent::ACTIVATED); });
}

void AplClientBridge::onActivityEnded(const std::string& token, const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, source] { m_guiManager->handleActivityEvent(source, ActivityEvent::DEACTIVATED); });
}

void AplClientBridge::onSendEvent(const std::string& token, const std::string& event) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, event] { m_guiManager->handleUserEvent(event); });
}

void AplClientBridge::onCommandExecutionComplete(const std::string& token, bool result) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, result] { m_guiManager->handleExecuteCommandsResult(token, result, ""); });
}

void AplClientBridge::onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, result, error] { m_guiManager->handleRenderDocumentResult(token, result, error); });
}

void AplClientBridge::onVisualContextAvailable(
    const std::string& token,
    unsigned int stateRequestToken,
    const std::string& context) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit(
        [this, stateRequestToken, context] { m_guiManager->handleVisualContext(stateRequestToken, context); });
}

void AplClientBridge::onSetDocumentIdleTimeout(const std::string& token, const std::chrono::milliseconds& timeout) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, timeout] { m_guiManager->setDocumentIdleTimeout(timeout); });
}

void AplClientBridge::onFinish(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this] { m_guiManager->forceExit(); });
}

void AplClientBridge::onRuntimeErrorEvent(const std::string& token, const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, payload] { m_guiManager->handleRuntimeErrorEvent(payload); });
}

void AplClientBridge::onDataSourceFetchRequestEvent(
    const std::string& token,
    const std::string& type,
    const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, type, payload] { m_guiManager->handleDataSourceFetchRequestEvent(type, payload); });
}

void AplClientBridge::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const std::string& source,
    const std::string& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, uri, name, source, params, event, resultCallback] {
        m_aplClientRenderer->onExtensionEvent(uri, name, source, params, event, resultCallback);
    });
}

void AplClientBridge::logMessage(APLClient::LogLevel level, const std::string& source, const std::string& message) {
    switch (level) {
        case APLClient::LogLevel::CRITICAL:
        case APLClient::LogLevel::ERROR:
            ACSDK_ERROR(LX(source).m(message));
            break;
        case APLClient::LogLevel::WARN:
            ACSDK_WARN(LX(source).m(message));
            break;
        case APLClient::LogLevel::INFO:
            ACSDK_INFO(LX(source).m(message));
            break;
        case APLClient::LogLevel::DBG:
            ACSDK_DEBUG0(LX(source).m(message));
            break;
        case APLClient::LogLevel::TRACE:
            ACSDK_DEBUG9(LX(source).m(message));
            break;
    }
}

void AplClientBridge::onConnectionOpened() {
    ACSDK_DEBUG9(LX("onConnectionOpened"));
    // Start the scheduled event timer to refresh the display at 60fps
    m_executor.submit([this] {
        m_updateTimer.start(
            std::chrono::milliseconds(16),
            Timer::PeriodType::ABSOLUTE,
            Timer::FOREVER,
            std::bind(&AplClientBridge::onUpdateTimer, this));
    });
}

void AplClientBridge::onConnectionClosed() {
    ACSDK_DEBUG9(LX("onConnectionClosed"));
    // Stop the outstanding timer as the client is no longer connected
    m_executor.submit([this] { m_updateTimer.stop(); });
}

void AplClientBridge::provideState(const unsigned int stateRequestToken) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, stateRequestToken] { m_aplClientRenderer->requestVisualContext(stateRequestToken); });
}

void AplClientBridge::onUpdateTimer() {
    bool renderQueued = m_renderQueued.exchange(true);
    if (renderQueued) {
        // Render was already queued, we can safely skip this rendering
        return;
    }

    m_executor.submit([this] {
        m_renderQueued = false;
        m_aplClientRenderer->onUpdateTick();
        if (m_audioPlayerExtension && m_guiManager &&
            alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING == m_playerActivityState) {
            m_audioPlayerExtension->updatePlaybackProgress(m_guiManager->getAudioItemOffset().count());
        }
    });
}

void AplClientBridge::setGUIManager(std::shared_ptr<GUIServerInterface> guiManager) {
    m_executor.submit([this, guiManager] { m_guiManager = guiManager; });
}

void AplClientBridge::renderDocument(
    const std::string& jsonPayload,
    const std::string& token,
    const std::string& windowId) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, jsonPayload, token, windowId] {
        m_windowId = windowId;

        rapidjson::Document document;
        if (document.Parse(jsonPayload).HasParseError()) {
            ACSDK_ERROR(LX("renderDocumentFailed").d("reason", "Failed to parse document"));
            m_guiManager->handleRenderDocumentResult(token, false, "Unable to create content");
            return;
        }

        // When rendering a new document, add the current active document state to backstack (if it should be cached)
        if (m_backstackExtension && m_backstackExtension->shouldCacheActiveDocument()) {
            if (auto documentState = m_aplClientRenderer->getActiveDocumentState()) {
                m_backstackExtension->addDocumentStateToBackstack(documentState);
            }
        }

        m_aplClientRenderer->renderDocument(
            extractDocument(document), extractData(document), extractSupportedViewports(document), token);
    });
}

void AplClientBridge::clearDocument() {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this] {
        m_aplClientRenderer->clearDocument("");
        if (m_backstackExtension) {
            m_backstackExtension->reset();
        }
    });
}

void AplClientBridge::executeCommands(const std::string& jsonPayload, const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, jsonPayload, token] { m_aplClientRenderer->executeCommands(jsonPayload, token); });
}

void AplClientBridge::interruptCommandSequence() {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this] { m_aplClientRenderer->interruptCommandSequence(); });
}

void AplClientBridge::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, sourceType, jsonPayload, token] {
        m_aplClientRenderer->dataSourceUpdate(sourceType, jsonPayload, token);
    });
}

void AplClientBridge::onMessage(const std::string& message) {
    ACSDK_DEBUG9(LX(__func__));

    if (m_aplClientRenderer->shouldHandleMessage(message)) {
        m_executor.submit([this, message] { m_aplClientRenderer->handleMessage(message); });
    }
}

bool AplClientBridge::handleBack() {
    if (m_backstackExtension) {
        return m_backstackExtension->handleBack();
    }
    return false;
}

void AplClientBridge::onPresentationSessionChanged() {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this] {
        if (m_backstackExtension) {
            m_backstackExtension->reset();
        }
    });
}

void AplClientBridge::onRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, event] { m_guiManager->handleAPLEvent(event); });
}

std::string AplClientBridge::extractDocument(const rapidjson::Document& document) {
    if (!document.HasMember("document")) {
        ACSDK_ERROR(LX("extractDocumentFailed").d("reason", "Failed to extract document"));
        return DEFAULT_PARAM_VALUE;
    }

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document["document"].Accept(writer);
    return sb.GetString();
}

std::string AplClientBridge::extractData(const rapidjson::Document& document) {
    if (!document.HasMember("datasources")) {
        ACSDK_WARN(LX("extractDataFailed").d("reason", "Failed to extract data"));
        return DEFAULT_PARAM_VALUE;
    }

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    document["datasources"].Accept(writer);
    return sb.GetString();
}

std::string AplClientBridge::extractSupportedViewports(const rapidjson::Document& document) {
    std::string jsonData;
    rapidjson::Value::ConstMemberIterator jsonIt;
    if (!document.HasMember("supportedViewports")) {
        ACSDK_WARN(LX("extractSupportedViewportsFailed").d("reason", "Failed to retrieve supportedViewports data"));
        jsonData = DEFAULT_PARAM_VALUE;
    } else {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        document["supportedViewports"].Accept(writer);
        jsonData = sb.GetString();
    }

    return jsonData;
}

int AplClientBridge::getMaxNumberOfConcurrentDownloads() {
    return m_parameters.maxNumberOfConcurrentDownloads;
}

void AplClientBridge::onRestoreDocumentState(std::shared_ptr<APLClient::AplDocumentState> documentState) {
    m_executor.submit([this, documentState]() { m_aplClientRenderer->restoreDocumentState(documentState); });
}

void AplClientBridge::onPlayerActivityChanged(
    alexaClientSDK::avsCommon::avs::PlayerActivity state,
    const Context& context) {
    m_executor.submit([this, state, context]() {
        m_playerActivityState = state;
        if (m_audioPlayerExtension) {
            m_audioPlayerExtension->updatePlayerActivity(
                playerActivityToString(m_playerActivityState), context.offset.count());
        }
    });
}

void AplClientBridge::onAudioPlayerPlay() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackPlay(); });
}

void AplClientBridge::onAudioPlayerPause() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackPause(); });
}

void AplClientBridge::onAudioPlayerNext() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackNext(); });
}

void AplClientBridge::onAudioPlayerPrevious() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackPrevious(); });
}

void AplClientBridge::onAudioPlayerSeekToPosition(int offsetInMilliseconds) {
    ACSDK_DEBUG3(LX(__func__));
}

void AplClientBridge::onAudioPlayerSkipForward() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackSkipForward(); });
}

void AplClientBridge::onAudioPlayerSkipBackward() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() { m_guiManager->handlePlaybackSkipBackward(); });
}

void AplClientBridge::onAudioPlayerToggle(const std::string& name, bool checked) {
    ACSDK_DEBUG3(LX(__func__).d("toggle", name).d("checked", checked));
    m_executor.submit([this, name, checked]() { m_guiManager->handlePlaybackToggle(name, checked); });
}

APLClient::AplRenderingEventObserverPtr AplClientBridge::getAplRenderingEventObserver() {
    return m_aplClientRenderer;
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK