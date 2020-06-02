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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCLIENTBINDING_H_
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCLIENTBINDING_H_

#include <memory>
#include <string>
#include <rapidjson/document.h>
#include "AplCoreConnectionManager.h"
#include "AplCoreGuiRenderer.h"
#include "AplOptionsInterface.h"

namespace APLClient {

/**
 * AplClientBinding abstracts away many of the implementation details of integrating with the APLCoreEngine and exposes
 * a smaller interface to allow rendering of APL documents on a remote view host through a client provided IPC layer.
 */
class AplClientBinding {
public:
    /**
     * Constructor
     */
    AplClientBinding(AplOptionsInterfacePtr options);

    virtual ~AplClientBinding() = default;

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

private:
    AplOptionsInterfacePtr m_aplOptions;

    AplCoreConnectionManagerPtr m_aplConnectionManager;

    std::unique_ptr<AplCoreGuiRenderer> m_aplGuiRenderer;
};
}  // namespace APLClient
#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCLIENTBINDING_H_
