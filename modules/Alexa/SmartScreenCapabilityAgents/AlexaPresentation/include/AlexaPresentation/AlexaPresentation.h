/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATION_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATION_H_

#include <chrono>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_set>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/AudioPlayerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>
#include <SmartScreenSDKInterfaces/VisualStateProviderInterface.h>

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

/**
 * This class implements a @c CapabilityAgent that handles the SS SDK @c AlexaPresentation API.  The
 * @c AlexaPresentation CA is responsible for handling the directives with Alexa.Presentation.APL namespace.
 *
 * The @c AlexaPresentation CA is also an observer to the @c DialogUXState to determine the end of a interaction so
 * that it would know when to clear a @c RenderDocument displayCard.
 *
 * The clients who are interested in any AlexaPresentation directives can subscribe themselves as an observer, and the
 * clients will be notified via the AlexaPresentationObserverInterface.
 */
class AlexaPresentation
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public std::enable_shared_from_this<AlexaPresentation> {
public:
    /**
     * Create an instance of @c AlexaPresentation.
     *
     * @param focusManager The object to use to fetch focus for the AlexaPresentation capabilityAgent.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @param messageSender The @c MessageSenderInterface that sends events to AVS.
     * @param contextManager The @c ContextManagerInterface used to generate system context for events.
     * @param visualStateProvider The @c VisualStateProviderInterface used to request visual context.
     * @return @c nullptr if the inputs are not defined, else a new instance of @c AlexaPresentation.
     */
    static std::shared_ptr<AlexaPresentation> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface>
            visualStateProvider = nullptr);

    /**
     * Destructor.
     */
    virtual ~AlexaPresentation() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}

    /// @name ChannelObserverInterface Functions
    /// @{
    void onFocusChanged(alexaClientSDK::avsCommon::avs::FocusState newFocus) override;
    /// @}

    /// @name DialogUXStateObserverInterface Functions
    /// @{
    void onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState newState) override;
    /// @}

    /// @name ContextRequesterInterface Functions
    /// @{
    void onContextAvailable(const std::string& jsonContext) override;
    void onContextFailure(const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestError error) override;
    /// @}

    /**
     * This function clear the card from the screen and also sends @c TemplateDismissed event to AVS along with
     * releasing any focus being held.
     */
    void clearCard();

    /**
     * This function adds an observer to @c AlexaPresentation so that it will get notified for all rendering
     * related directives.
     * @param observer The @c AlexaPresentationObserverInterface
     */
    void addObserver(std::shared_ptr<smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer);

    /**
     * This function removes an observer from @c AlexaPresentation so that it will no longer be notified of
     * rendering changes.
     *
     * @param observer The @c AlexaPresentationObserverInterface
     */
    void removeObserver(std::shared_ptr<smartScreenSDKInterfaces::AlexaPresentationObserverInterface> observer);

    /**
     * This function clear all pending @c ExecuteCommands directives and mark them as failed.
     */
    void clearAllExecuteCommands();

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /**
     * Send @c UserEvent to AVS
     *
     * @param payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendUserEvent(const std::string& payload);

    /// @name StateProviderInterface Functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        unsigned int stateRequestToken) override;
    /// @}

    /**
     * This function is called by the @c VisualContextProviderInterface with the visual context to be passed to AVS.
     * @param requestToken The token of the request for which this function is called.
     * @param payload The visual context payload to be passed to AVS.
     */
    void onVisualContextAvailable(const unsigned int requestToken, const std::string& payload);

    /**
     * Set The APL version supported by the runtime component
     * @param APLMaxVersion The APL version supported.
     * @note This function MUST be called before client connect flow.
     */
    void setAPLMaxVersion(const std::string& APLMaxVersion);

    /**
     * Set custom document timeout. Will be reset for every directive received.
     * @param timeout timeout in milliseconds.
     */
    void setDocumentIdleTimeout(std::chrono::milliseconds timeout);

    /**
     * Process result of RenderDocument directive.
     *
     * @param token document presentationToken.
     * @param result rendering result (true on rendered, false on exception).
     * @param error error message provided in case if result is false
     */
    void processRenderDocumentResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Process result of ExecuteCommands directive.
     *
     * @param token request token
     * @param result rendering result (true on executed, false on exception).
     * @param error error message provided in case if result is false
     */
    void processExecuteCommandsResult(const std::string& token, const bool result, const std::string& error);

    /**
     * Process activity change event from GUI Client.
     *
     * @param source The source of the activity event
     * @param event Activity change event.
     */
    void processActivityEvent(const std::string& source, const std::string& event);

    /**
     * Process activity change event.
     *
     * @param source The source of the activity event
     * @param event Activity change event.
     */
    void processActivityEvent(const std::string& source, const smartScreenSDKInterfaces::ActivityEvent event);

    /**
     * Set the executor used as the worker thread
     * @param executor The @c Executor to set
     * @note This function should only be used for testing purposes. No call to any other method should
     * be done prior to this call.
     */
    void setExecutor(const std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor>& executor);

private:
    /// Enum for the different events sent by the TemplateRuntime capability agent.
    enum class AlexaPresentationEvents {
        /// Event to be sent when an APL document is dimissed because of timeout or because of rendering a new screen.
        APL_DISMISSED,

        /**
         *  Event sent when user interacts with a rendered GUI component. The event carries the complete component
         * context of the render component in its payload.
         */
        APL_USER_EVENT
    };

    /// Document interaction state.
    enum class InteractionState {
        /// Interaction is going on.
        ACTIVE,

        /// No interaction happening.
        INACTIVE
    };

    /**
     * Constructor.
     *
     * @param focusManager The object to use to fetch focus for the TemplateRuntime capabilityAgent.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @param messageSender The object to send message to AVS.
     * @param contextManager The object to fetch the context of the system.
     * @param visualStateProvider The VisualStateProviderInterface object used to request visual context.
     */
    AlexaPresentation(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface>
            visualStateProvider = nullptr);

    // @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /// Initializes the object by reading the values from configuration.
    bool initialize();

    /**
     * Creates the Alexa.Presentation.APL interface configuration.
     *
     * @return The Alexa.Presentation.APL interface configuration.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
    getAlexaPresentationAPLCapabilityConfiguration();

    /**
     * Creates the Alexa.Presentation interface configuration.
     *
     * @return The Alexa.Presentation interface configuration.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
    getAlexaPresentationCapabilityConfiguration();

    /**
     * Remove a directive from the map of message IDs to DirectiveInfo instances.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective whose message ID is to be removed.
     */
    void removeDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * Send the handling completed notification and clean up the resources.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void setHandlingCompleted(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c RenderDocument directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleRenderDocumentDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c ExecuteCommand directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleExecuteCommandDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by @c AlexaPresentation CA.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function deserializes a @c Directive's payload into a @c rapidjson::Document.
     *
     * @param info The @c DirectiveInfo to read the payload string from.
     * @param[out] document The @c rapidjson::Document to parse the payload into.
     * @return @c true if parsing was successful, else @c false.
     */
    bool parseDirectivePayload(std::shared_ptr<DirectiveInfo> info, rapidjson::Document* document);

    /**
     * This function handles the notification of the renderDocument callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     */
    void executeRenderDocumentCallbacks(bool isClearCard);

    /**
     * This is an internal function that is called when the state machine is ready to notify the
     * @AlexaPresentation observers to display an APL document.
     */
    void executeDisplayCard();

    /**
     * This function handles the notification of the renderTemplateCard callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     *
     * @param info The directive to be handled.
     */
    void executeCommandEvent(std::shared_ptr<DirectiveInfo> info);

    /**
     * This is an internal function that is called when the state machine is ready to notify the
     * @AlexaPresentation observers to clear a card.
     */
    void executeClearCard();

    /**
     * This is an internal function to start the @c m_clearDisplayTimer.
     *
     * @param timeout The period of the timer.
     */
    void executeStartTimer(std::chrono::milliseconds timeout);

    /**
     * This is an internal function to restart the @c m_clearDisplayTimer.
     *
     * @param timeout The period of the timer.
     */
    void executeRestartTimer(std::chrono::milliseconds timeout);

    /**
     * This is an internal function to stop the @c m_clearDisplayTimer.
     */
    void executeStopTimer();

    /**
     * This is a state machine function to handle the clear card event.
     */
    void executeClearCardEvent();

    /**
     * This is a state machine function to handle the focus change event.
     */
    void executeOnFocusChangedEvent(alexaClientSDK::avsCommon::avs::FocusState newFocus);

    /**
     * This is a state machine function to handle the displayCard event.
     */
    void executeDisplayCardEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     * This is a state machine function to handle the execute command event.
     *
     * @param info The directive to be handled.
     */
    void executeExecuteCommandEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     *
     * Stops the execution of all pending @c ExecuteCommand directive
     */
    void executeClearAllExecuteCommands();

    /**
     *
     * Stops the execution of all pending @c ExecuteCommand directive
     */
    void doClearExecuteCommand(const std::string& reason);

    /**
     * Send APL Dismissed event to AVS.
     */
    void executeSendDismissedEvent();

    /**
     * Internal function for handling @c ContextManager request for context
     * @param stateRequestToken The request token.
     */
    void executeProvideState(unsigned int stateRequestToken);

    /**
     * Internal function for reseting the activity tracking state
     */
    void executeResetActivityTracker();

    /// Timer that is responsible for clearing the display on IDLE.
    alexaClientSDK::avsCommon::utils::timing::Timer m_idleTimer;

    /// Timer that is responsible for delayed execution.
    alexaClientSDK::avsCommon::utils::timing::Timer m_delayedExecutionTimer;

    /**
     * @name Executor Thread Variables
     *
     * These member variables are only accessed by functions in the @c m_executor worker thread, and do not require any
     * synchronization.
     */
    /// @{
    /// A set of observers to be notified when a @c RenderDocument/@ExecuteCommands directive is received.
    std::unordered_set<std::shared_ptr<smartScreenSDKInterfaces::AlexaPresentationObserverInterface>> m_observers;

    /// The directive corresponding to the RenderDocument directive.
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> m_lastDisplayedDirective;

    /// The last executeCommand directive.
    std::pair<std::string, std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo>>
        m_lastExecuteCommandTokenAndDirective;

    /// A flag indicating that a music update was received whilst activity tracking was active
    // bool m_musicActivityUpdateReceivedDuringActivity;

    /// The current focus state of the @c AlexaPresentation on the visual channel.
    alexaClientSDK::avsCommon::avs::FocusState m_focus;

    /// Interface that currently holds focus.
    std::string m_focusHoldingInterface;

    /// The state of the @c AlexaPresentation  state machine.
    smartScreenSDKInterfaces::State m_state;
    /// @}

    /// The current state of DialogUX
    DialogUXState m_dialogUxState;

    /// Set of sources which are currently reporting activity
    std::unordered_set<std::string> m_activeSources;
    /// @}

    /// The @c ContextManager used to fetch context of the system to be sent with events.

    /// The @c FocusManager used to manage usage of the visual channel.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// The timeout value in ms for clearing the display card when it is no interaction received from SDK config
    std::chrono::milliseconds m_sdkConfiguredDocumentInteractionTimeout;

    /// The timeout value in ms for clearing the display card when it is no interaction
    std::chrono::milliseconds m_documentInteractionTimeout;

    /// The object to use for sending events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// Token of the last template if it was an APL one. Otherwise, empty
    std::string m_lastRenderedAPLToken;

    /// The @c ContextManager used to generate system context for events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The @c VisualStateProvider for requesting visual state.
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> m_visualStateProvider;

    /// The queue of events to be sent to AVS.
    std::queue<std::pair<AlexaPresentationEvents, std::string>> m_events;

    /// The APL version of the runtime.
    std::string m_APLVersion;

    /// Current document interaction state.
    InteractionState m_documentInteractionState;

    /// This is the worker thread for the @c AlexaPresentation CA.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;
};

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATION_H_
