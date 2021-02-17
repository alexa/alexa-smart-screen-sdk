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

#ifdef ENABLE_APL_TELEMETRY
#include "SampleApp/TelemetrySink.h"
#endif

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

std::shared_ptr<AplClientBridge> AplClientBridge::create(
    std::shared_ptr<CachingDownloadManager> contentDownloadManager,
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
    AplClientBridgeParameter parameters) {
    std::shared_ptr<AplClientBridge> aplClientBridge(
        new AplClientBridge(contentDownloadManager, guiClient, parameters));
    aplClientBridge->m_aplClientBinding.reset(new APLClient::AplClientBinding(aplClientBridge));

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

void AplClientBridge::initializeRenderer(const std::string& windowId, std::set<std::string> supportedExtensions) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, windowId, supportedExtensions] {
        if (!windowId.empty()) {
            std::unordered_set<std::shared_ptr<APLClient::Extensions::AplCoreExtensionInterface>> extensions;
            for (auto& uri : supportedExtensions) {
                if (APLClient::Extensions::Backstack::URI == uri) {
                    extensions.emplace(std::make_shared<Backstack::AplBackstackExtension>(shared_from_this()));
                } else if (APLClient::Extensions::AudioPlayer::URI == uri) {
                    auto audioPlayerExtension =
                        std::make_shared<AudioPlayer::AplAudioPlayerExtension>(shared_from_this());
                    extensions.emplace(audioPlayerExtension);
                    m_audioPlayerExtensions.push_back(audioPlayerExtension);
                }
            }

            auto aplClientRenderer = m_aplClientBinding->createRenderer(windowId);
            aplClientRenderer->addExtensions(extensions);
            m_aplClientRendererMap[windowId] = aplClientRenderer;
        }
    });
}

void AplClientBridge::sendMessage(const std::string& token, const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));
    std::string newPayload = payload;
    auto aplClientRenderer = getAplClientRendererFromAplToken(token);
    if (aplClientRenderer) {
        auto aplCoreMessage = messages::AplCoreMessage(aplClientRenderer->getWindowId(), newPayload);
        m_guiClient->sendMessage(aplCoreMessage);
    }
}

void AplClientBridge::resetViewhost(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    auto aplClientRenderer = getAplClientRendererFromAplToken(token);
    if (aplClientRenderer) {
        auto message = messages::AplRenderMessage(aplClientRenderer->getWindowId(), token);
        m_guiClient->sendMessage(message);
    }
}

std::string AplClientBridge::downloadResource(const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    auto downloadMetricsEmitter = m_aplClientBinding->createDownloadMetricsEmitter();
    auto observer = std::make_shared<DownloadMonitor>(downloadMetricsEmitter);
    return m_contentDownloadManager->retrieveContent(source, observer);
}

std::chrono::milliseconds AplClientBridge::getTimezoneOffset() {
    // Relies on fact that this is always called in the executor thread via either RenderDocument or UpdateTimer
    return m_guiManager->getDeviceTimezoneOffset();
}

void AplClientBridge::onActivityStarted(const std::string& token, const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, source] { m_guiManager->handleActivityEvent(ActivityEvent::ACTIVATED, source); });
}

void AplClientBridge::onActivityEnded(const std::string& token, const std::string& source) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, source] { m_guiManager->handleActivityEvent(ActivityEvent::DEACTIVATED, source); });
}

void AplClientBridge::onSendEvent(const std::string& token, const std::string& event) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, event] { m_guiManager->handleUserEvent(token, event); });
}

void AplClientBridge::onCommandExecutionComplete(const std::string& token, bool result) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, result] { m_guiManager->handleExecuteCommandsResult(token, result, ""); });
}

void AplClientBridge::onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, result, error] {
        m_guiManager->handleRenderDocumentResult(token, result, error);

        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (!result && aplClientRenderer) {
            aplClientRenderer->onRenderingEvent(APLClient::AplRenderingEvent::RENDER_ABORTED);
        }
    });
}

void AplClientBridge::onVisualContextAvailable(
    const std::string& token,
    unsigned int stateRequestToken,
    const std::string& context) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, stateRequestToken, token, context] {
        m_guiManager->handleVisualContext(token, stateRequestToken, context);
    });
}

void AplClientBridge::onSetDocumentIdleTimeout(const std::string& token, const std::chrono::milliseconds& timeout) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, timeout] { m_guiManager->setDocumentIdleTimeout(token, timeout); });
}

void AplClientBridge::onFinish(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this] { m_guiManager->forceExit(); });
}

void AplClientBridge::onRuntimeErrorEvent(const std::string& token, const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, payload] { m_guiManager->handleRuntimeErrorEvent(token, payload); });
}

void AplClientBridge::onDataSourceFetchRequestEvent(
    const std::string& token,
    const std::string& type,
    const std::string& payload) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit(
        [this, token, type, payload] { m_guiManager->handleDataSourceFetchRequestEvent(token, type, payload); });
}

void AplClientBridge::onExtensionEvent(
    const std::string& aplToken,
    const std::string& uri,
    const std::string& name,
    const std::string& source,
    const std::string& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, aplToken, uri, name, source, params, event, resultCallback] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(aplToken);
        if (aplClientRenderer) {
            aplClientRenderer->onExtensionEvent(uri, name, source, params, event, resultCallback);
        }
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

void AplClientBridge::provideState(const std::string& aplToken, const unsigned int stateRequestToken) {
    ACSDK_DEBUG9(LX(__func__));

    m_executor.submit([this, stateRequestToken, aplToken] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(aplToken);
        if (aplClientRenderer) {
            aplClientRenderer->requestVisualContext(stateRequestToken);
        }
    });
}

void AplClientBridge::onUpdateTimer() {
    bool renderQueued = m_renderQueued.exchange(true);
    if (renderQueued) {
        // Render was already queued, we can safely skip this rendering
        return;
    }

    m_executor.submit([this] {
        m_renderQueued = false;
        for (auto aplClientRendererPair : m_aplClientRendererMap) {
            aplClientRendererPair.second->onUpdateTick();
        }

        if (m_guiManager && alexaClientSDK::avsCommon::avs::PlayerActivity::PLAYING == m_playerActivityState) {
            double audioItemOffset = m_guiManager->getAudioItemOffset().count();
            for (const auto& audioPlayerExtension : m_audioPlayerExtensions) {
                audioPlayerExtension->updatePlaybackProgress(audioItemOffset);
            }
        }
    });
}

void AplClientBridge::setGUIManager(std::shared_ptr<GUIServerInterface> guiManager) {
    m_executor.submit([this, guiManager] { m_guiManager = guiManager; });
}

void AplClientBridge::renderDocument(
    const std::string& token,
    const std::string& document,
    const std::string& dataSources,
    const std::string& supportedViewports,
    const std::string& windowId) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, document, dataSources, supportedViewports, windowId] {
        std::shared_ptr<APLClient::AplClientRenderer> aplClientRenderer;
        m_lastRenderedWindowId = windowId;
        if (m_aplClientRendererMap.find(windowId) != m_aplClientRendererMap.end()) {
            aplClientRenderer = m_aplClientRendererMap[windowId];

            std::string previouslyServingToken = aplClientRenderer->getCurrentAPLToken();
            m_aplTokenToWindowIdMap.erase(previouslyServingToken);
        } else {
            /// Will be reached for windowId not found in configurations
            aplClientRenderer = m_aplClientBinding->createRenderer(windowId);
            m_aplClientRendererMap[windowId] = aplClientRenderer;
        }

        setTokenToWindow(token, windowId);

        if (auto backExtension = getBackExtensionForRenderer(aplClientRenderer)) {
            if (backExtension->shouldCacheActiveDocument()) {
                if (auto documentState = aplClientRenderer->getActiveDocumentState()) {
                    backExtension->addDocumentStateToBackstack(documentState);
                }
            }
        }

        aplClientRenderer->renderDocument(document, dataSources, supportedViewports, token);
    });
}

void AplClientBridge::clearDocument(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        executeClearDocument(aplClientRenderer);
    });
}

void AplClientBridge::executeClearDocument(std::shared_ptr<APLClient::AplClientRenderer> aplClientRenderer) {
    ACSDK_DEBUG9(LX(__func__));
    if (aplClientRenderer) {
        std::string windowId = aplClientRenderer->getWindowId();

        std::string previouslyServingToken = aplClientRenderer->getCurrentAPLToken();
        m_aplTokenToWindowIdMap.erase(previouslyServingToken);
        aplClientRenderer->clearDocument();

        // Reset the render's backstack on document clear
        if (auto backExtension = getBackExtensionForRenderer(aplClientRenderer)) {
            backExtension->reset();
        }

        auto clearDocumentMessage = messages::ClearDocumentMessage(windowId);
        m_guiClient->sendMessage(clearDocumentMessage);
    }
}

void AplClientBridge::executeCommands(const std::string& jsonPayload, const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, jsonPayload, token] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (aplClientRenderer) {
            aplClientRenderer->executeCommands(jsonPayload, token);
        }
    });
}

void AplClientBridge::interruptCommandSequence(const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (aplClientRenderer) {
            aplClientRenderer->interruptCommandSequence();
        }
    });
}

void AplClientBridge::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, sourceType, jsonPayload, token] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (aplClientRenderer) {
            aplClientRenderer->dataSourceUpdate(sourceType, jsonPayload, token);
        }
    });
}

void AplClientBridge::onMessage(const std::string& windowId, const std::string& message) {
    ACSDK_DEBUG9(LX(__func__));

    auto aplClientRenderer = getAplClientRendererFromWindowId(windowId);
    if (aplClientRenderer && aplClientRenderer->shouldHandleMessage(message)) {
        m_executor.submit([message, aplClientRenderer] { aplClientRenderer->handleMessage(message); });
    }
}

bool AplClientBridge::handleBack() {
    return m_executor
        .submit([this] {
            if (auto aplClientRenderer = getAplClientRendererFromWindowId(m_lastRenderedWindowId)) {
                if (auto backExtension = getBackExtensionForRenderer(aplClientRenderer)) {
                    return backExtension->handleBack();
                }
            }
            return false;
        })
        .get();
}

void AplClientBridge::onPresentationSessionChanged(const std::string& id, const std::string& skillId) {
    ACSDK_DEBUG9(LX(__func__));
    // Reset the active window's backstack on session change
    if (auto aplClientRenderer = getAplClientRendererFromWindowId(m_lastRenderedWindowId)) {
        if (auto backExtension = getBackExtensionForRenderer(aplClientRenderer)) {
            backExtension->reset();
        }
    }

    // Notify all audio player extensions of presentation session change.
    for (const auto& audioPlayerExtension : m_audioPlayerExtensions) {
        audioPlayerExtension->setActivePresentationSession(id, skillId);
    }
}

void AplClientBridge::onRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, event] { m_guiManager->handleAPLEvent(event); });
}

int AplClientBridge::getMaxNumberOfConcurrentDownloads() {
    return m_parameters.maxNumberOfConcurrentDownloads;
}

void AplClientBridge::onRestoreDocumentState(std::shared_ptr<APLClient::AplDocumentState> documentState) {
    // We restore into the last rendered (active) window
    auto aplClientRenderer = getAplClientRendererFromWindowId(m_lastRenderedWindowId);
    if (aplClientRenderer) {
        // The restored document's token is now associated with the active renderer's window id
        setTokenToWindow(documentState->token, aplClientRenderer->getWindowId());
        return aplClientRenderer->restoreDocumentState(documentState);
    }
}

void AplClientBridge::onPlayerActivityChanged(
    alexaClientSDK::avsCommon::avs::PlayerActivity state,
    const Context& context) {
    m_executor.submit([this, state, context]() {
        m_playerActivityState = state;
        for (const auto& audioPlayerExtension : m_audioPlayerExtensions) {
            audioPlayerExtension->updatePlayerActivity(
                playerActivityToString(m_playerActivityState), context.offset.count());
        }
    });
}

std::shared_ptr<APLClient::Extensions::Backstack::AplBackstackExtension> AplClientBridge::getBackExtensionForRenderer(
    const std::shared_ptr<APLClient::AplClientRenderer>& aplClientRenderer) {
    return std::dynamic_pointer_cast<APLClient::Extensions::Backstack::AplBackstackExtension>(
        aplClientRenderer->getExtension(APLClient::Extensions::Backstack::URI));
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

void AplClientBridge::onMetricRecorderAvailable(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
#ifdef ENABLE_APL_TELEMETRY
    if (metricRecorder) {
        auto sink = std::make_shared<TelemetrySink>(metricRecorder);
        m_aplClientBinding->onTelemetrySinkUpdated(sink);
    }
#endif
}

void AplClientBridge::handleRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, event] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (aplClientRenderer) {
            aplClientRenderer->onRenderingEvent(event);
        }
    });
}

void AplClientBridge::handleDisplayMetrics(
    const std::string& windowId,
    const std::vector<APLClient::DisplayMetric>& metrics) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, windowId, metrics] {
        auto aplClientRenderer = getAplClientRendererFromWindowId(windowId);
        if (aplClientRenderer) {
            aplClientRenderer->onMetricsReported(metrics);
        }
    });
}

void AplClientBridge::onRenderDirectiveReceived(
    const std::string& token,
    const std::chrono::steady_clock::time_point& receiveTime) {
    ACSDK_DEBUG9(LX(__func__));
    m_executor.submit([this, token, receiveTime] {
        auto aplClientRenderer = getAplClientRendererFromAplToken(token);
        if (aplClientRenderer) {
            aplClientRenderer->onRenderDirectiveReceived(receiveTime);
        }
    });
}

std::shared_ptr<APLClient::AplClientRenderer> AplClientBridge::getAplClientRendererFromWindowId(
    const std::string& windowId) {
    std::shared_ptr<APLClient::AplClientRenderer> renderer = nullptr;
    auto m_aplClientRendererMapIter = m_aplClientRendererMap.find(windowId);
    if (m_aplClientRendererMapIter != m_aplClientRendererMap.end()) {
        renderer = m_aplClientRendererMapIter->second;
    }

    if (!renderer) {
        ACSDK_WARN(LX(__func__).d("targetWindowId", windowId).m("Unable to find renderer for this windowId"));
    }

    return renderer;
}

std::shared_ptr<APLClient::AplClientRenderer> AplClientBridge::getAplClientRendererFromAplToken(
    const std::string& aplToken) {
    std::shared_ptr<APLClient::AplClientRenderer> renderer = nullptr;
    auto aplTokenToWindowIdMapIter = m_aplTokenToWindowIdMap.find(aplToken);
    if (aplTokenToWindowIdMapIter != m_aplTokenToWindowIdMap.end()) {
        std::string windowId = aplTokenToWindowIdMapIter->second;

        auto m_aplClientRendererMapIter = m_aplClientRendererMap.find(windowId);
        if (m_aplClientRendererMapIter != m_aplClientRendererMap.end()) {
            renderer = m_aplClientRendererMapIter->second;
        }

        if (!renderer) {
            ACSDK_WARN(LX(__func__).d("APLToken", aplToken).m("Unable to find renderer for this token"));
        }
    }

    return renderer;
}

void AplClientBridge::setTokenToWindow(const std::string& token, const std::string& windowId) {
    m_aplTokenToWindowIdMap[token] = windowId;
}

/// TODO: Implement the onAudioPlayerLyricDataFlushed function from the @c AplAudioPlayerExtensionObserverInterface
void AplClientBridge::onAudioPlayerLyricDataFlushed(
    const std::string& token,
    long durationInMilliseconds,
    const std::string& lyricData) {
    ACSDK_DEBUG3(LX(__func__));
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK