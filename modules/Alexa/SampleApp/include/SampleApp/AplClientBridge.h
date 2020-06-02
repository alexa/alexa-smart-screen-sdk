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

#include <AVSCommon/Utils/LibcurlUtils/HTTPContentFetcherFactory.h>
#include "SmartScreenSDKInterfaces/MessagingServerObserverInterface.h"
#include "APLClient/AplClientBinding.h"
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
        , public smartScreenSDKInterfaces::VisualStateProviderInterface {
public:
    static std::shared_ptr<AplClientBridge> create(
        std::shared_ptr<CachingDownloadManager> contentDownloadManager,
        std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClient,
        AplClientBridgeParameter parameters);

    /// @name AplOptionsInterface Functions
    /// {
    void sendMessage(const std::string& payload) override;

    void resetViewhost(const std::string& token) override;

    std::string downloadResource(const std::string& source) override;

    std::chrono::milliseconds getTimezoneOffset() override;

    void onActivityStarted(const std::string& source) override;

    void onActivityEnded(const std::string& source) override;

    void onSendEvent(const std::string& event) override;

    void onCommandExecutionComplete(const std::string& token, bool result) override;

    void onRenderDocumentComplete(const std::string& token, bool result, const std::string& error) override;

    void onVisualContextAvailable(unsigned int stateRequestToken, const std::string& context) override;

    void onSetDocumentIdleTimeout(const std::chrono::milliseconds& timeout) override;

    void onRenderingEvent(APLClient::AplRenderingEvent event) override;

    void onFinish() override;

    void onDataSourceFetchRequestEvent(const std::string& type, const std::string& payload) override;

    void onRuntimeErrorEvent(const std::string& payload) override;

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

    void onUpdateTimer();

    void setGUIManager(std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface> guiManager);

    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId = "");

    void clearDocument();

    void executeCommands(const std::string& jsonPayload, const std::string& token);

    void interruptCommandSequence();

    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    void onMessage(const std::string& message);

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
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCLIENTBRIDGE_H_
