/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <tuple>
#include <unordered_set>
#include <utility>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <acsdkAudioPlayerInterfaces/AudioPlayerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>
#include <AVSCommon/Utils/Timing/TimerDelegateFactory.h>
#include <AVSCommon/Utils/Metrics/MetricRecorderInterface.h>
#include <AVSCommon/Utils/Metrics/DataPointDurationBuilder.h>
#include <AVSCommon/Utils/Metrics/DataPointCounterBuilder.h>
#include <AVSCommon/Utils/Optional.h>

#include <APLClient/AplRenderingEvent.h>
#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>
#include <SmartScreenSDKInterfaces/VisualStateProviderInterface.h>

#include "TimeoutType.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

/// Identifier for the presentationSession sent in a RenderDocument directive
static const char PRESENTATION_SESSION_FIELD[] = "presentationSession";

/// Identifier for the skilld in presentationSession
static const char SKILL_ID[] = "skillId";

/// Identifier for the id in presentationSession
static const char PRESENTATION_SESSION_ID[] = "id";

/// Identifier for the grantedExtensions in presentationSession
static const char PRESENTATION_SESSION_GRANTEDEXTENSIONS[] = "grantedExtensions";

/// Identifier for the autoInitializedExtensions in presentationSession
static const char PRESENTATION_SESSION_AUTOINITIALIZEDEXTENSIONS[] = "autoInitializedExtensions";

//// Identifier for the uri in grantedExtensions or autoInitializedExtensions
static const char PRESENTATION_SESSION_URI[] = "uri";

/// Identifier for the settings in autoInitializedExtensions
static const char PRESENTATION_SESSION_SETTINGS[] = "settings";

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
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface>
            visualStateProvider = nullptr,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::timing::TimerDelegateFactoryInterface>
            timerDelegateFactory = nullptr);

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

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name ChannelObserverInterface Functions
    /// @{
    void onFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior) override;
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

    /// @name StateProviderInterface Functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        unsigned int stateRequestToken) override;
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
     * @param token The token. This should be passed in if we are clearing execute commands due to APL-specific trigger
     * (eg. Finish command). This should be left empty if we are clearing due to global triggers (eg. back navigation)
     * @param markAsFailed Whether to mark the cleared commands as failed.
     */
    void clearExecuteCommands(const std::string& token = std::string(), const bool markAsFailed = true);

    /**
     * Send @c UserEvent to AVS
     *
     * @param payload The @c UserEvent event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendUserEvent(const std::string& payload);

    /**
     * Send @c DataSourceFetchRequest to AVS
     *
     * @param type type of DataSource requesting a fetch.
     * @param payload The @c DataSourceFetchRequest event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendDataSourceFetchRequestEvent(const std::string& type, const std::string& payload);

    /**
     * Send @c RuntimeError to AVS
     *
     * @param payload The @c RuntimeError event payload. The caller of this
     * function is responsible to pass the payload as it defined by AVS.
     */
    void sendRuntimeErrorEvent(const std::string& payload);

    /**
     * This function is called by the @c VisualContextProviderInterface with the visual context to be passed to AVS.
     * @param requestToken The token of the request for which this function is called.
     * @param visualContext The visual context visualContext to be passed to AVS.
     */
    void onVisualContextAvailable(const unsigned int requestToken, const std::string& visualContext);

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

    /**
     * Record the finish event for currently rendering document
     */
    void recordRenderComplete();

    /**
     * Record display Metrics event
     * @param dropFrameCount Count of the frames dropped
     */
    void recordDropFrameCount(uint64_t dropFrameCount);

    /**
     * Record the APL event for currently rendering document
     */
    void recordAPLEvent(APLClient::AplRenderingEvent event);

    /**
     * The placeholder token to use for rendering Non-APL documents
     */
    static const std::string getNonAPLDocumentToken();

private:
    /// Document interaction state.
    enum class InteractionState {
        /// Interaction is going on.
        ACTIVE,

        /// No interaction happening.
        INACTIVE
    };

    /**
     * Contains the values for the presentationSession object that is found in the
     * @c Alexa.Presentation.APL @c RenderDocument directive.
     */
    struct PresentationSession {
        /**
         * Default Constructor.
         */
        PresentationSession() = default;

        /**
         * Constructor.
         *
         * @param skillId the identifier of the skill/speechlet.
         * @param id The identifier of the presentation session.
         * @param grantedExtensions List of extensions that are granted for use by this APL document.
         * @param autoInitializedExtensions List of extensions that are initialized in the APL runtime for this
         * document.
         */
        PresentationSession(
            std::string skillId,
            std::string id,
            std::vector<smartScreenSDKInterfaces::GrantedExtension> grantedExtensions,
            std::vector<smartScreenSDKInterfaces::AutoInitializedExtension> autoInitializedExtensions) :
                skillId{std::move(skillId)},
                id{std::move(id)},
                grantedExtensions{std::move(grantedExtensions)},
                autoInitializedExtensions{std::move(autoInitializedExtensions)} {};

        /// The identifier of the Skill/ Speechlet who sends this directive.
        std::string skillId;

        /// The identifier of the presentation session.
        std::string id;

        // List of extensions that are granted for use by this APL document.
        std::vector<smartScreenSDKInterfaces::GrantedExtension> grantedExtensions;

        // List of extensions that are initialized in the APL runtime for this document.
        std::vector<smartScreenSDKInterfaces::AutoInitializedExtension> autoInitializedExtensions;

        bool operator==(const PresentationSession& other) const {
            return skillId == other.skillId && id == other.id && grantedExtensions == other.grantedExtensions &&
                   autoInitializedExtensions == other.autoInitializedExtensions;
        }

        bool operator!=(const PresentationSession& other) const {
            return skillId != other.skillId || id != other.id || grantedExtensions != other.grantedExtensions ||
                   autoInitializedExtensions != other.autoInitializedExtensions;
        }

        /**
         * Adds presentationSession payload to provided document.
         * @param document the document to add the payload to.
         */
        void addPresentationSessionPayload(rapidjson::Document* document) {
            rapidjson::Document::AllocatorType& allocator = document->GetAllocator();
            rapidjson::Document presentationSession(rapidjson::kObjectType, &allocator);
            presentationSession.AddMember(SKILL_ID, skillId, allocator);
            presentationSession.AddMember(PRESENTATION_SESSION_ID, id, allocator);

            rapidjson::Document grantedExtensionsDoc(rapidjson::kArrayType, &allocator);
            grantedExtensionsDoc.SetArray();
            for (auto grantedExtension : grantedExtensions) {
                rapidjson::Document doc(rapidjson::kObjectType, &allocator);
                doc.AddMember(PRESENTATION_SESSION_URI, grantedExtension.uri, allocator);
                grantedExtensionsDoc.PushBack(doc, allocator);
            }
            presentationSession.AddMember(PRESENTATION_SESSION_GRANTEDEXTENSIONS, grantedExtensionsDoc, allocator);

            rapidjson::Document autoInitializedExtensionsDoc(rapidjson::kArrayType, &allocator);
            autoInitializedExtensionsDoc.SetArray();
            for (auto autoInitializedExtension : autoInitializedExtensions) {
                rapidjson::Document doc(rapidjson::kObjectType, &allocator);
                doc.AddMember(PRESENTATION_SESSION_URI, autoInitializedExtension.uri, allocator);
                doc.AddMember(PRESENTATION_SESSION_SETTINGS, autoInitializedExtension.settings, allocator);
                autoInitializedExtensionsDoc.PushBack(doc, allocator);
            }
            presentationSession.AddMember(
                PRESENTATION_SESSION_AUTOINITIALIZEDEXTENSIONS, autoInitializedExtensionsDoc, allocator);

            document->AddMember(PRESENTATION_SESSION_FIELD, presentationSession, allocator);
        }

        /**
         * Returns string payload of presentationSession object.
         * @return the presentationSession object payload.
         */
        std::string getPresentationSessionPayload() {
            rapidjson::Document doc(rapidjson::kObjectType);
            addPresentationSessionPayload(&doc);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            return buffer.GetString();
        }
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
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface>
            visualStateProvider = nullptr,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::timing::TimerDelegateFactoryInterface>
            timerDelegateFactory = nullptr);

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
     * This function handles a @c Dynamic source data related directives.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     * @param sourceType Dynamic source type.
     */
    void handleDynamicListDataDirective(std::shared_ptr<DirectiveInfo> info, const std::string& sourceType);

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
    void executeRenderDocument();

    /**
     * This function handles the notification of the renderTemplateCard callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     *
     * @param info The directive to be handled.
     */
    void executeExecuteCommand(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles the notification of the processDataSourceUpdate callbacks to all the observers.  This
     * function is intended to be used in the context of @c m_executor worker thread.
     *
     * @param info The directive to be handled.
     * @param sourceType Data source type.
     */
    void executeDataSourceUpdate(std::shared_ptr<DirectiveInfo> info, const std::string& sourceType);

    /**
     * This is an internal function that is called when the state machine is ready to notify the
     * @AlexaPresentation observers to clear a card.
     */
    void executeClearCard();

    /**
     * This is an internal function to start or extend the @c m_idleTimer.
     */
    void executeStartOrExtendTimer();

    /**
     * This is an internal function to stop the @c m_idleTimer.
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
     * This is a state machine function to handle the renderDocument event.
     */
    void executeRenderDocumentEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     * This is a state machine function to handle the execute command event.
     *
     * @param info The directive to be handled.
     */
    void executeExecuteCommandEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     * This is a state machine function to handle the LoadIndexListData event.
     *
     * @param info The directive to be handled.
     * @param sourceType Data source type.
     */
    void executeDataSourceUpdateEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info,
        const std::string& sourceType);

    /**
     *
     * Stops the execution of all pending @c ExecuteCommand directive
     * @param reason reason for clearing commands
     * @param token The token. This should be passed in if we are clearing execute commands due to APL-specific trigger
     * (eg. Finish command). This should be left empty if we are clearing due to global triggers (eg. back navigation)
     * @param markAsFailed Whether to mark the cleared commands as failed.
     */
    void executeClearExecuteCommands(
        const std::string& reason,
        const std::string& token = std::string(),
        const bool markAsFailed = true);
    /**
     * Queue an AVS event to be sent when context is available
     * @param avsNamespace namespace of the event
     * @param name name of the event
     * @param payload payload of the event
     */
    void executeSendEvent(const std::string& avsNamespace, const std::string& name, const std::string& payload);

    /**
     * Internal function for handling @c ContextManager request for context
     * @param stateRequestToken The request token.
     */
    void executeProvideState(unsigned int stateRequestToken);

    /**
     * Internal function for reseting the activity tracking state
     */
    void executeResetActivityTracker();

    /**
     * Checks if a proactive state report is required and requests state if necessary
     */
    void executeProactiveStateReport();

    /**
     * Request a proactive state report on the appropriate thread
     */
    void proactiveStateReport();

    /**
     * Extract skill id from APL token
     */
    std::string getSkillIdFromAPLToken(const std::string& aplToken);

    /**
     * Notify all observers that rendering has been aborted.
     */
    void notifyAbort();

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

    /// The @c FocusManager used to manage usage of the visual channel.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// The timeout value in ms for clearing the display card when it is no interaction
    alexaClientSDK::avsCommon::utils::Optional<std::chrono::milliseconds> m_documentInteractionTimeout;

    /// The object to use for sending events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// Window id of the last rendered window if it was an APL one. Otherwise, empty.
    std::string m_lastTargetedWindowId;

    /// Token of the last template if it was an APL one. Otherwise, empty
    std::string m_lastRenderedAPLToken;

    /// The @c ContextManager used to generate system context for events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The @c VisualStateProvider for requesting visual state.
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface> m_visualStateProvider;

    /// The queue of events to be sent to AVS.
    std::queue<std::tuple<std::string, std::string, std::string>> m_events;

    /// The APL version of the runtime.
    std::string m_APLVersion;

    /// Current document interaction state.
    InteractionState m_documentInteractionState;

    /// @{
    /// Helper members required for metrics collection

    /// Enumeration of timer metrics events that could be emitted from @c SmartScreenClient.
    enum class MetricEvent {
        /// Metric to record time-taken to render document
        RENDER_DOCUMENT,

        /// Metric to record time-taken to render document by view-host
        LAYOUT,

        /// Metric to record time-taken to document inflate event
        INFLATE,

        /// Metric to count the number of times text measurement was initiated
        TEXT_MEASURE_COUNT,

        /// Metric to count number of dropped frames
        DROP_FRAME,

        /// Out of Bound
        MAX
    };

    /// Metrics DataPoint Names
    static std::map<MetricEvent, std::string> MetricsDataPointNames;

    /**
     * Start recording or update @c metricsEvent
     *
     * @param Metrics event in context
     */
    void startMetricsEvent(MetricEvent metricEvent);

    /**
     * Stops recording and submit @c metricsEvent
     *
     * @param metricsEvent Metrics event in context
     * @param activityName Name of the activity to be concluded
     */
    void endMetricsEvent(MetricEvent metricEvent, const std::string& activityName);

    /**
     * Reset @c metricsEvent
     *
     * @param metricsEvent Metrics event in context
     */
    void resetMetricsEvent(MetricEvent metricEvent);

    /**
     * Records a single metrics data-point with value and submit @c metricsEvent
     *
     * @param metricsEvent Metrics event in context
     * @param count Total Count of the event
     * @param activityName Name of the activity to be concluded
     */
    void triggerMetricsEventWithData(MetricEvent metricEvent, uint64_t count, const std::string& activityName);

    /**
     * Records a single metrics data-point with value and submit @c metricsEvent
     *
     * @param metricsEvent Metrics event in context
     * @param tp Start-time of the event
     * @param activityName Name of the activity to be concluded
     */
    void triggerMetricsEventWithData(
        MetricEvent metricEvent,
        std::chrono::steady_clock::time_point& tp,
        const std::string& activityName);

    /// The @c MetricRecorder used to record useful metrics from the presentation layer.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> m_metricRecorder;

    /// The mutex to ensure exclusivity over @c MetricRecorder
    std::mutex m_MetricsRecorderMutex;

    /// Stores the currently active time data points
    std::map<MetricEvent, std::chrono::steady_clock::time_point> m_currentActiveTimePoints;

    /// Stores the currently active count data points
    std::map<MetricEvent, uint64_t> m_currentActiveCountPoints;
    /// @}

    /// The last state which was reported to AVS
    std::string m_lastReportedState;

    /// The time of the last state report
    std::chrono::time_point<std::chrono::steady_clock> m_lastReportTime;

    /// The minimum state reporting interval
    std::chrono::milliseconds m_minStateReportInterval{};

    /// The state reporting check interval
    std::chrono::milliseconds m_stateReportCheckInterval{};

    /// Whether the state has been requested from the state provider and we are awaiting the response
    bool m_stateReportPending;

    /// An internal timer used to check for context changes
    alexaClientSDK::avsCommon::utils::timing::Timer m_proactiveStateTimer;

    /// Whether the current document is fully rendered
    bool m_documentRendered;

    /// The current @c PresentationSession as set by the latest @c RenderDocument directive.
    PresentationSession m_presentationSession;

    /// This is the worker thread for the @c AlexaPresentation CA.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;

    /// Time at which the current document was received
    std::chrono::steady_clock::time_point m_renderReceivedTime;
};

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_ALEXAPRESENTATION_H_
