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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCORECONNECTIONMANAGER_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCORECONNECTIONMANAGER_H

#include <string>

#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>
// TODO: Tidy up core to prevent this (ARC-917)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/apl.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop

#include "AplCoreViewhostMessage.h"
#include "AplCoreMetrics.h"
#include "AplOptionsInterface.h"

namespace APLClient {

/**
 * Interacts with the APL Core Engine handling the event loop, updates etc. and passes messages between the core
 * and the viewhost.
 */
class AplCoreConnectionManager : public std::enable_shared_from_this<AplCoreConnectionManager> {
public:
    /**
     * Constructor
     * @param guiClientInterface Pointer to the GUI Client interface
     */
    AplCoreConnectionManager(const AplOptionsInterfacePtr aplOptions);

    virtual ~AplCoreConnectionManager() = default;

public:
    /**
     * Sets the APL Content to be rendered by the APL Core
     * @param content
     * @param token APL Presentation token for this content
     */
    void setContent(const apl::ContentPtr content, const std::string& token);

    /**
     * Sets the APL ScalingOptions
     * @param supportedViewports The JSON Payload
     */
    void setSupportedViewports(const std::string& jsonPayload);

    /**
     * Receives messages from the APL view host and identifies if it will require further handling
     * @note This function does not need to be handled on the same execution thread as other function calls
     * @param message The JSON Payload
     * @return true if the message should be passed to @c handleMessage, false if message
     */
    bool shouldHandleMessage(const std::string& message);

    /**
     * Receives messages from the APL view host
     * @param message The JSON Payload
     */
    void handleMessage(const std::string& message);

    /**
     * Executes an APL command
     * @param command The command to execute
     * @param token Directive token to bind result processing
     */
    void executeCommands(const std::string& command, const std::string& token);

    /**
     * Execute DataSource updates.
     * @param sourceType DataSource type.
     * @param jsonPayload The payload of the directive in structured JSON format.
     * @param token Directive token used to bind result processing.
     */
    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token);

    /**
     * Interrupts the currently executing APL command sequence
     */
    void interruptCommandSequence();

    /**
     * Send a message to the view host and block until you get a reply
     * @param message The message to send
     * @return The resultant message or a NULL object if a response was not received.
     */
    rapidjson::Document blockingSend(
        AplCoreViewhostMessage& message,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(2000));

    void provideState(unsigned int stateRequestToken);

    AplCoreMetricsPtr aplCoreMetrics() const {
        return m_AplCoreMetrics;
    }

    /**
     * Schedules an update on the root context and runs the update loop - this may result in the viewhost being
     * updated and any events currently pending will be processed. If nothing is currently being displayed calling
     * this method will result in no action being taken. For the best rendering experience it is recommended that this
     * method is scheduled to be called at a rate which matches the refresh rate of the users display.
     */
    void onUpdateTick();

    /**
     * Resets the connection manager to remove the current document
     */
    void reset();

private:
    /**
     * Sends document theme information to the client
     */
    void sendDocumentThemeMessage();

    /**
     * Sends document background information to the client
     * @param background
     */
    void sendDocumentBackgroundMessage(const apl::Object& background);

    /**
     * Sends screenLock state to the client
     * @param screenLock
     */
    void sendScreenLockMessage(bool screenLock);

    /**
     * Handles the build message received from the view host, builds the component hierarchy.
     * @param message
     */
    void handleBuild(const rapidjson::Value& message);

    /**
     * Handle an update message from the view host of the form:
     *
     *     { "id": COMPONENT_ID, "type": EventType(int), "value": Integer }
     *
     * @param update
     */
    void handleUpdate(const rapidjson::Value& update);

    /**
     * Handle an media update message from the view host of the form:
     *
     *     { "id": COMPONENT_ID, "mediaState": apl::MediaState, "fromEvent": boolean }
     *
     * @param update
     */
    void handleMediaUpdate(const rapidjson::Value& update);

    /**
     * Handle an graphic update message from the view host of the form:
     *
     *     { "id": COMPONENT_ID, "avg": json }
     *
     * @param update
     */
    void handleGraphicUpdate(const rapidjson::Value& update);

    /**
     * Handles the ensureLayout message received from the viewhost
     * @param payload
     */
    void handleEnsureLayout(const rapidjson::Value& payload);

    /**
     * Handle the scrollToRectInComponent message received from the viewhost
     * @param payload
     */
    void handleScrollToRectInComponent(const rapidjson::Value& payload);

    /**
     * Handle the handleKeyboard message received from the viewhost
     * @param payload
     */
    void handleHandleKeyboard(const rapidjson::Value& payload);

    /**
     * Handle the updateCursorPosition message received from the viewhost
     * @param payload
     */
    void handleUpdateCursorPosition(const rapidjson::Value& payload);

    /**
     * Process responses to events with action references.  The payload should be of the form:
     *
     *    { "event": EVENT_NUMBER, "argument": VALUE }
     *
     * @param response
     */
    void handleEventResponse(const rapidjson::Value& response);

    /**
     * Check for screenLock condition and process it accordingly.
     */
    void handleScreenLock();

    /**
     * Execute the event.
     * ActionRefs have to be stored while we are waiting for a response.
     * Terminates have to be sent up if the action is cancelled.
     * Resolves/Rejects sent down have to be acted up.
     * @param event requested core event.
     */
    void processEvent(const apl::Event& event);

    /**
     * Process set of dirty components and send out dirty properties as required.
     * @param dirty dirty components set.
     */
    void processDirty(const std::set<apl::ComponentPtr>& dirty);

    /**
     * APL Core relies on operations to be performed in particular way.
     * Order and set of operations in this method should be preserved.
     * Order is the following:
     * * Update time and adjust TimeZone if required.
     * * Call **clearPending** method on RootConfig to give Core possibility to execute all pending actions and updates.
     * * Process requested events.      * * Process dirty properties.
     * * Check and set screenlock if required.
     */
    void coreFrameUpdate();

    /**
     * Send a message to the view host
     * @param message The message to send
     * @return The sequence number of this message
     */
    unsigned int send(AplCoreViewhostMessage& message);

    /**
     * Sends an error message to the view host
     * @param message The message to send to the view hsot
     */
    void sendError(const std::string& message);

    /**
     * Retrieves the current time
     * @return The time
     */
    std::chrono::milliseconds getCurrentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    }

    /**
     * Get optional value from Json.
     * @param jsonNode json data
     * @param key Member name
     * @param defaultValue Default value
     * @return Value from json or default if not found.
     */
    double getOptionalValue(const rapidjson::Value& jsonNode, const std::string& key, double defaultValue);

    /**
     * Get optional value from Json.
     * @param jsonNode json data
     * @param key Member name
     * @param defaultValue Default value
     * @return Value from json or default if not found.
     */
    std::string getOptionalValue(
        const rapidjson::Value& jsonNode,
        const std::string& key,
        const std::string& defaultValue);

    /**
     * Get optional bool value from Json.
     * @param jsonNode json data
     * @param key Member name
     * @param defaultValue Default value
     * @return Value from json or default if not found.
     */
    bool getOptionalBool(const rapidjson::Value& jsonNode, const std::string& key, bool defaultValue);

    /**
     * Get optional integer value from Json.
     * @param jsonNode json data
     * @param key Member name
     * @param defaultValue Default value
     * @return Value from json or default if not found.
     */
    int getOptionalInt(const rapidjson::Value& jsonNode, const std::string& key, int defaultValue);

    /**
     * Gets a rect from Json and converts it to an apl::Rect
     * @param jsonNode json data
     * @return An apl rect with the values from the json
     */
    apl::Rect convertJsonToScaledRect(const rapidjson::Value& jsonNode);

    /**
     * Check if any errors returned from any of loaded datasources and report them.
     */
    void checkAndSendDataSourceErrors();

    /// View host message type to handler map
    std::map<std::string, std::function<void(const rapidjson::Value&)>> m_messageHandlers;

    /// Shared pointer to the APL Content
    apl::ContentPtr m_Content;

    /// The APL presentation token for the currently rendered document
    std::string m_aplToken;

    /// The APL Metrics object - received from the view host and used for generating the apl root context
    apl::Metrics m_Metrics;

    /**
     * The Viewport Size Specifications object - created on directive processing and passed to core in
     * order to calculate scaling.
     */
    std::vector<apl::ViewportSpecification> m_ViewportSizeSpecifications;

    /**
     * Scaling calculation object.
     */
    AplCoreMetricsPtr m_AplCoreMetrics;

    /// Pointer to the APL Root Context
    apl::RootContextPtr m_Root;

    /// Map of pending APL Core events
    std::map<int, apl::ActionRef> m_PendingEvents;

    /// The start time used to calculate the update time used by APL Core
    std::chrono::milliseconds m_StartTime;

    /// Pointer to APL Options
    AplOptionsInterfacePtr m_aplOptions;

    /// Screen lock flag
    bool m_ScreenLock;

    /// Next packet sequence number
    unsigned int m_SequenceNumber;

    /// The sequence number which a blockingSend is waiting for
    unsigned int m_replyExpectedSequenceNumber;

    /// Whether we are expecting a reply to a blockingSend
    bool m_blockingSendReplyExpected;

    /// The pending promise from a call to blockingSend
    std::promise<std::string> m_replyPromise;

    /// The mutex protecting blockingSend
    std::mutex m_blockingSendMutex;
};

using AplCoreConnectionManagerPtr = std::shared_ptr<AplCoreConnectionManager>;

}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCORECONNECTIONMANAGER_H
