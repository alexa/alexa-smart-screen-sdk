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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIME_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIME_H_

#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsObserverInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderInterface.h>
#include <AVSCommon/SDKInterfaces/RenderPlayerInfoCardsProviderRegistrarInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include "SmartScreenSDKInterfaces/ActivityEvent.h"
#include "SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h"
#include "SmartScreenSDKInterfaces/AudioPlayerInfo.h"
#include "SmartScreenSDKInterfaces/DisplayCardState.h"
#include "SmartScreenSDKInterfaces/TemplateRuntimeObserverInterface.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace templateRuntime {

/**
 * This class implements a @c CapabilityAgent that handles the AVS @c TemplateRuntime API.  The
 * @c TemplateRuntime CA is responsible for handling the directives with the TemplateRuntime namespace.  Due
 * to the fact that the @c RenderPlayerInfo directives are closely related to the @c AudioPlayer, the @c TemplateRuntime
 * CA is an observer to the AudioPlayer and will be synchronizing the @c RenderPlayerInfo directives with the
 * corresponding @c AudioItem being handled in the @c AudioPlayer.
 *
 * The @c TemplateRuntime CA is also an observer to the @c DialogUXState to determine the end of a interaction so
 * that it would know when to clear a @c RenderTemplate displayCard.
 *
 * The clients who are interested in any TemplateRuntime directives can subscribe themselves as an observer, and the
 * clients will be notified via the TemplateRuntimeObserverInterface.
 */
class TemplateRuntime
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface
        , public std::enable_shared_from_this<TemplateRuntime> {
public:
    /**
     * Create an instance of @c TemplateRuntime.
     *
     * @param renderPlayerInfoCardsProviderRegistrar The registrar containing the set of @c
     * RenderPlayerInfoCardsProviders.
     * @param focusManager The object to use for acquire and release focus.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @return @c nullptr if the inputs are not defined, else a new instance of @c TemplateRuntime.
     */
    static std::shared_ptr<TemplateRuntime> createTemplateRuntime(
        const std::shared_ptr<
            alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderRegistrarInterface>&
            renderPlayerInfoCardsInterfaces,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    /**
     * Create an instance of @c TemplateRuntime.
     *
     * @param renderPlayerInfoCardsInterfaces A set of objects to use for subscribing @c TemplateRuntime as an
     * observer of changes for RenderPlayerInfoCards.
     * @param focusManager The object to use for acquire and release focus.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @return @c nullptr if the inputs are not defined, else a new instance of @c TemplateRuntime.
     */
    static std::shared_ptr<TemplateRuntime> create(
        const std::unordered_set<
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>&
            renderPlayerInfoCardsInterfaces,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    /**
     * Destructor.
     */
    virtual ~TemplateRuntime() = default;

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
    void onFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior) override;
    /// @}

    /// @name RenderPlayerInfoCardsObserverInterface Functions
    /// @{
    void onRenderPlayerCardsInfoChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context)
        override;
    /// @}

    /// @name DialogUXStateObserverInterface Functions
    /// @{
    void onDialogUXStateChanged(
        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState newState) override;
    /// @}

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name AlexaPresentationObserverInterface Functions
    /// @{
    void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) override;

    void clearDocument(const std::string& token, const bool focusCleared) override;

    void executeCommands(const std::string& jsonPayload, const std::string& token) override;

    void dataSourceUpdate(const std::string& sourceType, const std::string& jsonPayload, const std::string& token)
        override;

    void interruptCommandSequence(const std::string& token) override;

    void onPresentationSessionChanged(
        const std::string& id,
        const std::string& skillId,
        const std::vector<smartScreenSDKInterfaces::GrantedExtension>& grantedExtensions,
        const std::vector<smartScreenSDKInterfaces::AutoInitializedExtension>& autoInitializedExtensions) override;
    /// @}

    /**
     * This function adds an observer to @c TemplateRuntime so that it will get notified for renderTemplateCard or
     * renderPlayerInfoCard.
     *
     * @param observer The @c TemplateRuntimeObserverInterface
     */
    void addObserver(std::shared_ptr<smartScreenSDKInterfaces::TemplateRuntimeObserverInterface> observer);

    /**
     * This function removes an observer from @c TemplateRuntime so that it will no longer be notified of
     * renderTemplateCard or renderPlayerInfoCard callbacks.
     *
     * @param observer The @c TemplateRuntimeObserverInterface
     */
    void removeObserver(std::shared_ptr<smartScreenSDKInterfaces::TemplateRuntimeObserverInterface> observer);

    /**
     * This function notifies the @c TemplateRuntime that a displayCard has been cleared from the screen.  Upon getting
     * this notification, the @c TemplateRuntime will release the visual channel.
     */
    void displayCardCleared();

    /**
     * This function clears the displayed card from the screen and releases any focus being held.
     */
    void clearCard();

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
     * Process result of renderTemplate OR renderPlayerInfo directive.
     *
     * @param token document presentationToken.
     */
    void processPresentationResult(const std::string& token);

private:
    /**
     * Utility structure to correspond a directive with its audioItemId.
     */
    struct AudioItemPair {
        /**
         * Default Constructor.
         */
        AudioItemPair() = default;

        /**
         * Constructor.
         *
         * @param itemId The ID for the @c AudioItem.
         * @param renderPlayerInfoDirective The @c RenderPlayerInfo directive that corresponds to the audioItemId.
         */
        AudioItemPair(
            std::string itemId,
            std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> renderPlayerInfoDirective) :
                audioItemId{itemId}, directive{renderPlayerInfoDirective} {};

        /// The ID of the @c AudioItem.
        std::string audioItemId;

        /// The directive corresponding to the audioItemId.
        std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> directive;
    };

    /**
     * Constructor.
     *
     * @param renderPlayerInfoCardsInterfaces A set of objects to use for subscribing @c TemplateRuntime as an
     * observer of changes for RenderPlayerInfoCards.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     */
    TemplateRuntime(
        const std::unordered_set<
            std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>&
            renderPlayerInfoCardsInterfaces,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    // @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Initializes the object by reading the values from configuration.
     */
    bool initialize();

    /**
     * Creates the TemplateRuntime interface configuration.
     *
     * @return The TemplateRuntime interface configuration.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
    getTemplateRuntimeCapabilityConfiguration();

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
     * This function handles a @c RenderTemplate directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleRenderTemplateDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c RenderPlayerInfo directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleRenderPlayerInfoDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by @c TemplateRuntime CA.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This is an internal function that handles any card clear events for all non player info cards.
     *
     * @param clearedNonPlayerInfoDisplayType The @c NonPlayerInfoDisplayType of the cleared non player info card.
     */
    void executeNonPlayerInfoCardCleared(
        smartScreenSDKInterfaces::NonPlayerInfoDisplayType clearedNonPlayerInfoDisplayType);

    /**
     * This is an internal function that handles updating the @c m_audioItemInExecution when the @c AudioPlayer
     * notifies the @c TemplateRuntime CA of any changes in the @c AudioPlayer audio state.  This function is
     * intended to be used in the context of @c m_executor worker thread.
     *
     * @param state The @c PlayerActivity of the @c AudioPlayer.
     * @param context The @c Context of the @c AudioPlayer at the time of the notification.
     */
    void executeAudioPlayerInfoUpdates(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context);

    /**
     * This is an internal function that start or stop the @c m_clearDisplayTimer based on the @c PlayerActivity
     * reported by the @c AudioPlayer.
     *
     * @param state The @c PlayerActivity of the @c AudioPlayer.
     */
    void executeAudioPlayerStartTimer(alexaClientSDK::avsCommon::avs::PlayerActivity state);

    /**
     * This function handles the notification of the renderPlayerInfoCard callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     */
    void executeRenderPlayerInfoCallbacks(bool isClearCard);

    /**
     * This function handles the notification of the renderTemplateCard callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     */
    void executeRenderTemplateCallbacks(bool isClearCard);

    /**
     * This is an internal function that is called when the state machine is ready to notify the @TemplateRuntime
     * observers to display a card.
     */
    void executeDisplayCard();

    /**
     * This is an internal function that is called when the state machine is ready to notify the @TemplateRuntime
     * observers to clear a card.
     */
    void executeClearCard();
    /**
     * This is an internal function to restart the @c m_clearDisplayTimer.
     *
     * @param timeout The period of the timer.
     */
    void executeRestartTimer(std::chrono::milliseconds timeout);

    /**
     * This is an internal function to start the @c m_clearDisplayTimer.
     *
     * @param timeout The period of the timer.
     */
    void executeStartTimer(std::chrono::milliseconds timeout);

    /**
     * This is an internal function to stop the @c m_clearDisplayTimer.
     */
    void executeStopTimer();

    /**
     * This is a state machine function to handle the clear card event.
     * Called when agent has been asked to clear the displayed card.
     */
    void executeClearCardEvent();

    /**
     * This is a state machine function to handle the card cleared event.
     * Called when agent has been notified that a card was cleared.
     */
    void executeCardClearedEvent();

    /**
     * This is a state machine function to handle the focus change event.
     */
    void executeOnFocusChangedEvent(alexaClientSDK::avsCommon::avs::FocusState newFocus);

    /**
     * This is a state machine function to handle the displayCard event.
     */
    void executeDisplayCardEvent(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /// Timer that is responsible for clearing the display.
    alexaClientSDK::avsCommon::utils::timing::Timer m_clearDisplayTimer;

    /**
     * @name Executor Thread Variables
     *
     * These member variables are only accessed by functions in the @c m_executor worker thread, and do not require any
     * synchronization.
     */
    /// @{
    /// A set of observers to be notified when a @c RenderTemplate or @c RenderPlayerInfo directive is received
    std::unordered_set<std::shared_ptr<smartScreenSDKInterfaces::TemplateRuntimeObserverInterface>> m_observers;

    /*
     * This is a map that is used to store the current executing @c AudioItem based on the callbacks from the
     * @c RenderPlayerInfoCardsProviderInterface.
     */
    std::unordered_map<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface>,
        AudioItemPair>
        m_audioItemsInExecution;

    /// The current active RenderPlayerInfoCards provider that has the matching audioItemId.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface>
        m_activeRenderPlayerInfoCardsProvider;

    /*
     * This queue is for storing the @c RenderPlayerInfo directives when its audioItemId does not match the audioItemId
     * in execution in the @c AudioPlayer.
     */
    std::deque<AudioItemPair> m_audioItems;

    /// This map is to store the @c AudioPlayerInfo to be passed to the observers in the renderPlayerInfoCard callback.
    std::unordered_map<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface>,
        smartScreenSDKInterfaces::AudioPlayerInfo>
        m_audioPlayerInfo;

    /// The directive corresponding to the RenderTemplate directive.
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> m_lastDisplayedDirective;

    /// The currently active smartScreenSDKInterfaces::NonPlayerInfoDisplayType
    smartScreenSDKInterfaces::NonPlayerInfoDisplayType m_activeNonPlayerInfoType;

    /// The current focus state of the @c TemplateRuntime on the visual channel.
    alexaClientSDK::avsCommon::avs::FocusState m_focus;

    /// The state of the capability agent.
    smartScreenSDKInterfaces::State m_state;
    /// @}

    /// The @c PlayerActivity of the @c AudioPlayer
    alexaClientSDK::avsCommon::avs::PlayerActivity m_playerActivityState;
    /// @}

    /*
     * This is a set of interfaces to the @c RenderPlayerInfoCardsProviderInterface.  The @c TemplateRuntime CA
     * used this interface to add and remove itself as an observer.
     */
    std::unordered_set<
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface>>
        m_renderPlayerInfoCardsInterfaces;

    /// The @c FocusManager used to manage usage of the visual channel.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// The timeout value in ms for clearing the diplay card when TTS is FINSIHED
    std::chrono::milliseconds m_ttsFinishedTimeout{};

    /// The timeout value in ms for clearing the diplay card when AudioPlay is FINSIHED
    std::chrono::milliseconds m_audioPlaybackFinishedTimeout{};

    /// The timeout value in ms for clearing the diplay card when AudioPlay is STOPPED or PAUSED
    std::chrono::milliseconds m_audioPlaybackStoppedPausedTimeout{};

    /// The timeout value in ms for clearing the display card when it is no interaction
    std::chrono::milliseconds m_templateCardInteractionTimeout{};

    /**
     * The @c Executor which queues up operations from asynchronous API calls.
     *
     * @note This declaration needs to come *after* the Executor Thread Variables above so that the thread shuts down
     *     before the Executor Thread Variables are destroyed.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;

    std::string m_playerInfoCardToken;
    std::string m_nonPlayerInfoCardToken;
};

}  // namespace templateRuntime
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIME_H_
