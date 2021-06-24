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

#ifndef APL_CLIENT_LIBRARY_APL_CORE_GUI_RENDERER_H_
#define APL_CLIENT_LIBRARY_APL_CORE_GUI_RENDERER_H_

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

#include "AplConfiguration.h"
#include "AplCoreConnectionManager.h"

namespace APLClient {

/**
 * Handles the initial creation of the APL content and retrieves package dependencies, also handles interaction with
 * the @c AplCoreConnectionManager
 */
class AplCoreGuiRenderer {
public:
    AplCoreGuiRenderer(AplConfigurationPtr config, AplCoreConnectionManagerPtr aplCoreConnectionManager);

    /**
     * Renders the given template document and data payload through Apl Core
     * @param document Template
     * @param data Payload
     * @param supportedViewports SupportedViewports
     * @param token The token for APL payload, empty string otherwise
     */
    void renderDocument(
        const std::string& document,
        const std::string& data,
        const std::string& supportedViewports,
        const std::string& token);

    /**
     * Clears the currently rendered document
     *
     */
    void clearDocument();

    /**
     * Executes the given sequence of APL commands
     * @param jsonPayload The APL commands to execute
     * @param token The APL token
     */
    void executeCommands(const std::string& jsonPayload, const std::string& token);

    /**
     * For lazy loading - updates the data source which is used by the currently rendered document
     * @param sourceType The data source type
     * @param jsonPayload The new data source payload
     * @param token The APL token
     */
    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    /**
     * Interrupts the currently executing command sequence
     */
    void interruptCommandSequence();

private:
    AplConfigurationPtr m_aplConfiguration;

    /**
     * A reference to the APL Core connection manager to forward APL messages to
     */
    AplCoreConnectionManagerPtr m_aplCoreConnectionManager;

    /**
     * A flag indicating if the document has been cleared.
     * Used to cover the gap in time between request to render and any incoming clear events.
     */
    bool m_isDocumentCleared;
};
}  // namespace APLClient

#endif  // APL_CLIENT_LIBRARY_APL_CORE_GUI_RENDERER_H_
