/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCOREGUIRENDERER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCOREGUIRENDERER_H_

// TODO: Tidy up core to prevent this (ARC-917)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/content/content.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop
#include <thread>

#include <SmartScreenSDKInterfaces/AudioPlayerInfo.h>
#include <SmartScreenSDKInterfaces/GUIServerInterface.h>
#include <SmartScreenSDKInterfaces/TemplateRuntimeObserverInterface.h>

#include "SampleApp/AplCoreConnectionManager.h"
#include "SampleApp/AplCoreGuiContentDownloadManager.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * Emits TemplateRuntime payloads as JSON with a header into a given MessagingInterface
 */
class AplCoreGuiRenderer {
public:
    /**
     * Constructor.
     *
     * @param messagingInterface Pointer to the messaging interface
     * @param aplCoreConnectionManager Pointer to the APL Core connection manager
     * @param aplCoreGuiContentDownloadManager Pointer to an APL Content Download Manager
     */
    AplCoreGuiRenderer(
        std::shared_ptr<AplCoreConnectionManager> aplCoreConnectionManager,
        std::shared_ptr<AplCoreGuiContentDownloadManager> aplCoreGuiContentDownloadManager);

    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId = "");

    void clearDocument();

    void executeCommands(const std::string& jsonPayload, const std::string& token);

    void interruptCommandSequence();

    void setGuiManager(const std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> guiManager);

private:
    /**
     * Renders the given template document and data payload through Apl Core
     * @param document Template
     * @param data Payload
     * @param supportedViewports SupportedViewports
     * @param token The token for APL payload, empty string otherwise
     * @param windowId The target windowId
     */
    void renderByAplCore(
        const std::string& document,
        const std::string& data,
        const std::string& supportedViewports,
        const std::string& token,
        const std::string& windowId);

    /**
     * A flag indicating if the document has been cleared.
     * Used to cover the gap in time between request to render and any incoming clear events.
     */
    bool m_isDocumentCleared;

    /**
     * Extracts the document section from an APL payload
     * @param jsonPayload
     * @return The extracted document
     */
    std::string extractDocument(const std::string& jsonPayload);

    /**
     * Extracts the data section from an APL payload
     * @param jsonPayload
     * @return The extracted data
     */
    std::string extractData(const std::string& jsonPayload);

    /**
     * Extracts the SupportedViewports section from a directive
     * @param jsonPayload
     * @return The extracted section
     */
    std::string extractSupportedViewports(const std::string& jsonPayload);

    /**
     * A reference to the APL Core connection manager to forward APL messages to
     */
    std::shared_ptr<AplCoreConnectionManager> m_aplCoreConnectionManager;

    /**
     * A reference to the Apl Core Gui Content Download manager to get packages
     */
    std::shared_ptr<AplCoreGuiContentDownloadManager> m_aplCoreGuiContentDownloadManager;

    /**
     * A reference to GUIManager
     */
    std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> m_guiManager;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCOREGUIRENDERER_H_
