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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCLIENTBRIDGE_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCLIENTBRIDGE_H_

#include <acsdkAudioPlayerInterfaces/AudioPlayerObserverInterface.h>
#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>

#include "SmartScreenSDKInterfaces/MessagingServerObserverInterface.h"
#include "APLClient/AplClientBinding.h"
#include "APLClient/AplRenderingEventObserver.h"
#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include <APLClient/Extensions/Backstack/AplBackstackExtension.h>
#include <APLClient/Extensions/Backstack/AplBackstackExtensionObserver.h>
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h>
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerExtensionObserverInterface.h>
#include <APLClient/AplRenderingEvent.h>
#include "GUI/GUIManager.h"
#include "CachingDownloadManager.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * A struct that helps storing additional parameters for APLClientBridge.
 */
struct AplClientBridgeParameter {
    // Maximum number of concurrent downloads allowed.
    int maxNumberOfConcurrentDownloads;
};

class AplClientBridge
        : public APLClient::AplOptionsInterface
        , public smartScreenSDKInterfaces::MessagingServerObserverInterface
        , public smartScreenSDKInterfaces::VisualStateProviderInterface
        , public alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface
        , public APLClient::Extensions::Backstack::AplBackstackExtensionObserverInterface
        , public APLClient::Extensions::AudioPlayer::AplAudioPlayerExtensionObserverInterface
        , public std::enable_shared_from_this<AplClientBridge> {
public:
    static std::shared_ptr<AplClientBridge> create(
        std::shared_ptr<CachingDownloadManager> contentDownloadManager,
        std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
        AplClientBridgeParameter parameters);

    /// @name AplOptionsInterface Functions
    /// {
    void sendMessage(const std::string& token, const std::string& payload) override;

    void resetViewhost(const std::string& token) override;

    std::string downloadResource(const std::string& source) override;

    std::chrono::milliseconds getTimezoneOffset() override;

    void onActivityStarted(const std::string& token, const std::string& source) override;

    void onActivityEnded(const std::string& token, const std::string& source) override;

    void onSendEvent(const std::string& token, const std::string& event) override;

    void onCommandExecutionComplete(const std::string& token, bool result) override;

    void onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) override;

    void onVisualContextAvailable(const std::string& token, unsigned int stateRequestToken, const std::string& context)
        override;

    void onSetDocumentIdleTimeout(const std::string& token, const std::chrono::milliseconds& timeout) override;

    void onRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event) override;

    void onFinish(const std::string& token) override;

    void onDataSourceFetchRequestEvent(const std::string& token, const std::string& type, const std::string& payload)
        override;

    void onRuntimeErrorEvent(const std::string& token, const std::string& payload) override;

    void onExtensionEvent(
        const std::string& aplToken,
        const std::string& uri,
        const std::string& name,
        const std::string& source,
        const std::string& params,
        unsigned int event,
        std::shared_ptr<APLClient::Extensions::AplCoreExtensionEventCallbackResultInterface> resultCallback) override;

    void logMessage(APLClient::LogLevel level, const std::string& source, const std::string& message) override;

    int getMaxNumberOfConcurrentDownloads() override;

    /// }

    /// @name MessagingServerObserverInterface Functions
    /// @{
    void onConnectionOpened() override;

    void onConnectionClosed() override;
    /// @}

    /// @name VisualStateProviderInterface Methods
    /// @{
    void provideState(const std::string& aplToken, const unsigned int stateRequestToken) override;
    /// @}

    /// @name AudioPlayerObserverInterface methods
    /// @{
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    /// }

    /// @name AplBackstackExtensionObserverInterface Functions
    /// @{
    void onRestoreDocumentState(std::shared_ptr<APLClient::AplDocumentState> documentState) override;
    /// @}

    /// @name AplAudioPlayerExtensionObserverInterface methods
    /// @{
    void onAudioPlayerPlay() override;

    void onAudioPlayerPause() override;

    void onAudioPlayerNext() override;

    void onAudioPlayerPrevious() override;

    void onAudioPlayerSeekToPosition(int offsetInMilliseconds) override;

    void onAudioPlayerToggle(const std::string& name, bool checked) override;

    void onAudioPlayerSkipForward() override;

    void onAudioPlayerSkipBackward() override;

    void onAudioPlayerLyricDataFlushed(
        const std::string& token,
        long durationInMilliseconds,
        const std::string& lyricData) override;
    /// }

    void onUpdateTimer();

    void setGUIManager(std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface> guiManager);

    void renderDocument(
        const std::string& token,
        const std::string& document,
        const std::string& datasources,
        const std::string& supportedViewports,
        const std::string& windowId);

    void clearDocument(const std::string& token);

    void executeCommands(const std::string& jsonPayload, const std::string& token);

    void interruptCommandSequence(const std::string& token);

    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    void onMessage(const std::string& windowId, const std::string& message);

    bool handleBack();

    void onPresentationSessionChanged(const std::string& id, const std::string& skillId);

    void handleRenderingEvent(const std::string& token, APLClient::AplRenderingEvent event);

    void handleDisplayMetrics(const std::string& windowId, const std::vector<APLClient::DisplayMetric>& metrics);

    void onRenderDirectiveReceived(const std::string& token, const std::chrono::steady_clock::time_point& receiveTime);

    /**
     *  Initialize empty client renderer and load corresponding supported extensions
     *  @param windowId id of the window to be created
     *  @param supportedExtensions URIs of all supported APL extensions for this window
     */
    void initializeRenderer(const std::string& windowId, std::set<std::string> supportedExtensions);

    /**
     * Returns a shared pointer to the @c AplClientRenderer holding root-context for a given aplToken
     * Note:- This is not a thread safe method, avoid calling this method outside @c executor context
     *
     * @param the APL token in context
     * @return the instance of @c APLClientRenderer if found, else nullptr
     */
    std::shared_ptr<APLClient::AplClientRenderer> getAplClientRendererFromAplToken(const std::string& aplToken);

    /**
     * Returns a shared pointer to the @c AplClientRenderer holding root-context for a target window ID
     * Note:- This is not a thread safe method, avoid calling this method outside @c executor context
     *
     * @param the window id in context
     * @return the instance of @c APLClientRenderer if found, else nullptr
     */
    std::shared_ptr<APLClient::AplClientRenderer> getAplClientRendererFromWindowId(const std::string& windowId);

    /**
     * Sets the @TelemetrySink to @c AplConfiguration. This sink will be used by @c APLClient
     * to record and emit metric events.
     *
     * @param metricRecorder Shared Pointer to @MetricRecorderInterface to be used by @c TelemetrySink
     */
    void onMetricRecorderAvailable(
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder);

private:
    AplClientBridge(
        std::shared_ptr<CachingDownloadManager> contentDownloadManager,
        std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
        AplClientBridgeParameter parameters);

    /**
     * Set token to window id in the managed m_aplTokenToWindowIdMap
     * @param token of the apl document.
     * @param windowId the id of the window presenting the document with provided token.
     */
    void setTokenToWindow(const std::string& token, const std::string& windowId);

    /**
     * Executor method for clearing document, must be called in executor context.
     *
     * @param aplClientRenderer shared pointer of @c APLClientRenderer where the document is rendering
     */
    void executeClearDocument(std::shared_ptr<APLClient::AplClientRenderer> aplClientRenderer);

    /**
     * Gets the back extension associated with the provided renderer.
     *
     * @param aplClientRenderer shared pointer of @c APLClientRenderer to check for associated backstack extension.
     * @return Shared pointer to the back extension instance if available, else nullptr
     */
    static std::shared_ptr<APLClient::Extensions::Backstack::AplBackstackExtension> getBackExtensionForRenderer(
        const std::shared_ptr<APLClient::AplClientRenderer>& aplClientRenderer);

    /// Pointer to the download manager for retrieving resources
    std::shared_ptr<CachingDownloadManager> m_contentDownloadManager;

    /// An internal timer use to run the APL Core update loop
    alexaClientSDK::avsCommon::utils::timing::Timer m_updateTimer;

    /// Pointer to the APL Client
    std::unique_ptr<APLClient::AplClientBinding> m_aplClientBinding;

    /// Pointer to the GUI Manager
    std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> m_guiManager;

    /// Pointer to the GUI Client
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> m_guiClient;

    /// The last windowId to receive a RenderDocument directive
    std::string m_lastRenderedWindowId;

    /// Whether a render is currently queued
    std::atomic_bool m_renderQueued;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// An internal struct that stores additional parameters for AplClientBridge.
    AplClientBridgeParameter m_parameters;

    /// Collection of all @c AudioPlayerExtensions
    std::vector<std::shared_ptr<APLClient::Extensions::AudioPlayer::AplAudioPlayerExtension>> m_audioPlayerExtensions;

    /// The @c PlayerActivity state of the @c AudioPlayer
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerActivityState;

    /// Collection of Pointer to the @c AplClientRenderer for every @c windowId
    std::unordered_map<std::string, std::shared_ptr<APLClient::AplClientRenderer>> m_aplClientRendererMap;

    /// Map for resolving target @windowId currently rendering a given @c aplToken
    std::unordered_map<std::string, std::string> m_aplTokenToWindowIdMap;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCLIENTBRIDGE_H_
