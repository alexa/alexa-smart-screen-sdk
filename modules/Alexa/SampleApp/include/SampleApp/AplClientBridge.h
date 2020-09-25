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

#include "SmartScreenSDKInterfaces/MessagingServerObserverInterface.h"
#include "APLClient/AplClientBinding.h"
#include "APLClient/AplRenderingEventObserver.h"
#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h>
#include <APLClient/Extensions/AudioPlayer/AplAudioPlayerExtensionObserverInterface.h>
#include <APLClient/Extensions/Backstack/AplBackstackExtension.h>
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
        , public APLClient::Extensions::AudioPlayer::AplAudioPlayerExtensionObserverInterface
        , public APLClient::Extensions::Backstack::AplBackstackExtensionObserverInterface
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
    void provideState(const unsigned int stateRequestToken) override;
    /// @}

    /// @name AudioPlayerObserverInterface methods
    /// @{
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;
    /// }

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
    /// }

    /// @name AplBackstackExtensionObserverInterface Functions
    /// @{
    void onRestoreDocumentState(std::shared_ptr<APLClient::AplDocumentState> documentState) override;
    /// @}

    void onUpdateTimer();

    void setGUIManager(std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface> guiManager);

    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId = "");

    void clearDocument();

    void executeCommands(const std::string& jsonPayload, const std::string& token);

    void interruptCommandSequence();

    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    void onMessage(const std::string& message);

    bool handleBack();

    void onPresentationSessionChanged();

    /**
     * Loads default extensions managed by the @c AplClientBridge
     */
    void loadExtensions();

    /**
     *  Adds @c AplCoreExtensionInterface extensions to be registered with the @c AplClient
     * @param extensions Set of pointers to @c AplCoreExtensionInterface extensions.
     */
    void addExtensions(
        std::unordered_set<std::shared_ptr<APLClient::Extensions::AplCoreExtensionInterface>> extensions);

    /**
     * Returns a observer to be notified of rendering events.
     *
     * @return the observer
     */
    APLClient::AplRenderingEventObserverPtr getAplRenderingEventObserver();

private:
    AplClientBridge(
        std::shared_ptr<CachingDownloadManager> contentDownloadManager,
        std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,

        AplClientBridgeParameter parameters);

    /**
     * Extracts the document section from an APL payload
     * @param jsonPayload
     * @return The extracted document
     */
    std::string extractDocument(const rapidjson::Document& document);

    /**
     * Extracts the data section from an APL payload
     * @param jsonPayload
     * @return The extracted data
     */
    std::string extractData(const rapidjson::Document& document);

    /**
     * Extracts the SupportedViewports section from a directive
     * @param jsonPayload
     * @return The extracted section
     */
    std::string extractSupportedViewports(const rapidjson::Document& jsonPayload);

    /// Pointer to the download manager for retrieving resources
    std::shared_ptr<CachingDownloadManager> m_contentDownloadManager;

    /// An internal timer use to run the APL Core update loop
    alexaClientSDK::avsCommon::utils::timing::Timer m_updateTimer;

    /// Pointer to the APL Client
    std::unique_ptr<APLClient::AplClientBinding> m_aplClient;

    /// Pointer to the APL Client Renderer
    std::shared_ptr<APLClient::AplClientRenderer> m_aplClientRenderer;

    /// Pointer to the GUI Manager
    std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> m_guiManager;

    /// Pointer to the GUI Client
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> m_guiClient;

    /// The currently targeted window ID
    std::string m_windowId;

    /// Whether a render is currently queued
    std::atomic_bool m_renderQueued;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // An internal struct that stores additional parameters for AplClientBridge.
    AplClientBridgeParameter m_parameters;

    /// Pointer to the @c AplAudioPlayerExtension
    APLClient::Extensions::AudioPlayer::AplAudioPlayerExtensionPtr m_audioPlayerExtension;

    /// Pointer to the @c AplBackstackExtension
    APLClient::Extensions::Backstack::AplBackstackExtensionPtr m_backstackExtension;

    /// The @c PlayerActivity state of the @c AudioPlayer
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerActivityState;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCLIENTBRIDGE_H_
