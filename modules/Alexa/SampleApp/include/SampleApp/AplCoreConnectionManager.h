/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORECONNECTIONMANAGER_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORECONNECTIONMANAGER_H

#include <string>

#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>
#include <apl/apl.h>

#include <SmartScreenClient/SmartScreenClient.h>
#include <SmartScreenSDKInterfaces/GUIClientInterface.h>
#include <SmartScreenSDKInterfaces/GUIServerInterface.h>
#include <SmartScreenSDKInterfaces/MessagingServerObserverInterface.h>
#include <SmartScreenSDKInterfaces/VisualStateProviderInterface.h>

#include "Messages/AplCoreViewhostMessage.h"
#include "Messages/GUIClientMessage.h"
#include "AplCoreMetrics.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

class AplCoreConnectionManager
        : public smartScreenSDKInterfaces::MessagingServerObserverInterface
        , public std::enable_shared_from_this<AplCoreConnectionManager>
        , public smartScreenSDKInterfaces::VisualStateProviderInterface {
public:
    /**
     * Constructor
     * @param guiClientInterface Pointer to the GUI Client interface
     */
    AplCoreConnectionManager(const std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClientInterface);

    virtual ~AplCoreConnectionManager() = default;

    void setGuiManager(const std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> guiManager);

public:
    /**
     * Sets the APL Content to be rendered by the APL Core
     * @param content
     * @param token APL Presentation token for this content
     * @param windowId the target windowId
     */
    void setContent(const apl::ContentPtr content, const std::string& token, const std::string& windowId);

    /**
     * Sets the APL ScalingOptions
     * @param supportedViewports The JSON Payload
     */
    void setSupportedViewports(const std::string& jsonPayload);

    /**
     * Receives messages from the APL view host
     * @param message The JSON Payload
     */
    void onMessage(const std::string& message);

    /**
     * Executes an APL command
     * @param command The command to execute
     * @param token Directive token to bind result processing
     */
    void executeCommands(const std::string& command, const std::string& token);

    /**
     * Interrupts the currently executing APL command sequence
     */
    void interruptCommandSequence();

    /// @name MessagingServerObserverInterface Functions
    /// @{
    void onConnectionOpened() override;

    void onConnectionClosed() override;
    /// @}

    /**
     * Send a message to the view host and block until you get a reply
     * @param message The message to send
     * @return The resultant message or a NULL object if a response was not received.
     */
    rapidjson::Document blockingSend(
        messages::AplCoreViewhostMessage& message,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(2000));

    /// @name VisualStateProviderInterface Methods
    /// @{
    void provideState(const unsigned int stateRequestToken) override;
    /// @}

    AplCoreMetrics* aplCoreMetrics() const {
        return m_AplCoreMetrics;
    }

private:
    /**
     * Sends document theme information to the client
     */
    void sendDocumentThemeMessage();

    /**
     * Schedules an update on the root context and runs the update loop
     */
    void onUpdateTimer();

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

    // Execute the event loop until no more events are pending
    // ActionRefs have to be stored while we are waiting for a response.
    // Terminates have to be sent up if the action is cancelled.
    // Resolves/Rejects sent down have to be acted up.
    /**
     * Executes the event loop until no more events are pending in the APL Core.
     */
    void runEventLoop();

    /**
     * Send a message to the view host
     * @param message The message to send
     * @return The sequence number of this message
     */
    unsigned int send(messages::AplCoreViewhostMessage& message);

    /**
     * Sends an error message to the view host
     * @param message The message to send to the view hsot
     */
    void sendError(const std::string& message);

    /**
     * This function provides updated context information for @c AplCoreConnectionManager to @c ContextManager.  This
     * function is called when @c ContextManager calls @c provideState(), and is also called internally by @c
     * changeActivity().
     *
     * @param stateRequestToken The token @c ContextManager passed to the @c provideState() call, which will be passed
     *     along to the ContextManager::setState() call.
     */
    void executeProvideState(unsigned int stateRequestToken);

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
    AplCoreMetrics* m_AplCoreMetrics;

    /// Pointer to the APL Root Context
    apl::RootContextPtr m_Root;

    /// Map of pending APL Core events
    std::map<int, apl::ActionRef> m_PendingEvents;

    /// The start time used to calculate the update time used by APL Core
    std::chrono::milliseconds m_StartTime;

    /// Pointer to the GUI Client interface
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> m_guiClientInterface;

    /// Pointer to the GUI Manager interface
    std::shared_ptr<smartScreenSDKInterfaces::GUIServerInterface> m_guiManager;

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

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// An internal timer use to run the APL Core update loop
    alexaClientSDK::avsCommon::utils::timing::Timer m_updateTimer;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORECONNECTIONMANAGER_H
