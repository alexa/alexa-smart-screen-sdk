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

#ifndef APL_CLIENT_LIBRARY_APL_CLIENT_RENDERER_H_
#define APL_CLIENT_LIBRARY_APL_CLIENT_RENDERER_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <rapidjson/document.h>

#include "AplConfiguration.h"
#include "AplCoreConnectionManager.h"
#include "AplCoreGuiRenderer.h"
#include "AplRenderingEventObserver.h"
#include "AplRenderingEvent.h"
#include "Telemetry/AplMetricsRecorderInterface.h"
#include "APLClient/Extensions/Backstack/AplBackstackExtension.h"

namespace APLClient {
/**
 * @c AplClientRenderer abstracts away many of the implementation details of integrating with the @c APLCoreEngine and
 * exposes a smaller interface to allow rendering of APL documents on a remote view host through a client provided IPC
 * layer. The instance of this class represent a renderer targeting a single window. Therefore, the lifecycle of
 * instance of this class will be managed for every active render.
 */
class AplClientRenderer
        : public AplRenderingEventObserver
        , public std::enable_shared_from_this<AplClientRenderer> {
public:
    /**
     * Constructor
     */
    AplClientRenderer(AplConfigurationPtr config, std::string windowId);

    ~AplClientRenderer() override = default;

    /**
     * Pass a message received from the viewhost to the @c AplClientBinding, this should be called before
     * @c handleMessage and on a different thread to @c renderDocument.
     * @note This is a workaround to allow support for devices which do not support synchronous sends
     *
     * @param message
     * @return true if the message should be passed onwards to handleMessage, false if handling is complete
     */
    bool shouldHandleMessage(const std::string& message);

    /**
     * Pass a message received from the viewhost to the @c AplClientBinding, should only be called if
     * @c shouldHandleMessage returns true and must be run on the same thread as @c renderDocument
     * @param message The message from the viewhost
     */
    void handleMessage(const std::string& message);

    /**
     * Render an APL document
     * @param document The document json payload
     * @param data The document data
     * @param viewports The supported viewports
     * @param token The APL document token
     */
    void renderDocument(
        const std::string& document,
        const std::string& data,
        const std::string& viewports,
        const std::string& token);

    /**
     * Clears the current APL document
     */
    void clearDocument();

    /**
     * Execute an APL command sequence
     * @param jsonPayload The JSON APL command payload
     * @param token The APL document token
     */
    void executeCommands(const std::string& jsonPayload, const std::string& token);

    /**
     * Interrupts the currently executing command sequence
     */
    void interruptCommandSequence();

    /**
     * Requests the visual context
     */
    void requestVisualContext(unsigned int stateRequestToken);

    /**
     * Updates the data source
     * @param sourceType The source type
     * @param jsonPayload The json payload containing the new data
     * @param token The APL token
     */
    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    /**
     * Updates the rendered document
     * @note Ideally this function should should be called once for each screen refresh (e.g. 60 times per second)
     */
    void onUpdateTick();

    /**
     * Returns the target window Id for this renderer
     */
    const std::string getWindowId();

    /**
     * Returns the APL token currently served by this renderer
     */
    const std::string getCurrentAPLToken();

    /**
     * Adds Extensions to the client
     * @param extensions Set of Shared Pointers to AplCoreExtensionInterfaces
     */
    void addExtensions(std::unordered_set<std::shared_ptr<AplCoreExtensionInterface>> extensions);

    /**
     * Extension Event Callback function
     * @param uri Extension uri
     * @param name Extension event name
     * @param source Map of the source object that raised the event
     * @param params Map of the user-specified properties
     * @param event Event number
     * @param resultCallback Pointer to result callback interface
     */
    void onExtensionEvent(
        const std::string& uri,
        const std::string& name,
        const std::string& source,
        const std::string& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback);

    /**
     * Retrieve the active @c AplDocumentState.
     * @return The active @c AplDocumentState.
     */
    AplDocumentStatePtr getActiveDocumentState();

    /**
     * Restore content from provided @c AplDocumentState
     * @param documentState the @c AplDocumentState to restore.
     */
    void restoreDocumentState(AplDocumentStatePtr documentState);

    /// @name AplRenderingEventObserver Functions
    /// @{
    void onRenderDirectiveReceived(const std::chrono::steady_clock::time_point &receiveTime) override;
    void onRenderingEvent(AplRenderingEvent event) override;
    void onMetricsReported(const std::string& jsonPayload) override;
    void onTelemetrySinkUpdated(APLClient::Telemetry::AplMetricsSinkInterfacePtr sink) override;
    /// @}

    /**
     * Gets the instance of extension if supported
     *
     * @param uri URI of the extension requested
     * @return Pointer to the @c AplCoreExtensionInterface for extension if supported, else nullptr
     */
    std::shared_ptr<APLClient::Extensions::AplCoreExtensionInterface> getExtension(const std::string& uri);

private:
    AplConfigurationPtr m_aplConfiguration;

    std::string m_windowId;

    std::string m_aplToken;

    AplCoreConnectionManagerPtr m_aplConnectionManager;

    std::unique_ptr<AplCoreGuiRenderer> m_aplGuiRenderer;

    uint64_t m_lastReportedComplexity;

    std::unique_ptr<Telemetry::AplTimerHandle> m_renderTimer;

    /**
     * Validates the content from the metrics payload
     * 
     * @param jsonMetric The metric item to validate
     * @return bool Indicates whether the metric content is valid
     */
    bool validateJsonMetric(const rapidjson::Value& jsonMetric);
};

using AplClientRendererPtr = std::shared_ptr<AplClientRenderer>;

}  // namespace APLClient
#endif  // APL_CLIENT_LIBRARY_APL_CLIENT_RENDERER_H_
