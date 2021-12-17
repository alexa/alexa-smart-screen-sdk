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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENT_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENT_H_

#include <AVSCommon/AVS/AVSDirective.h>
#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <RTCSCNativeInterface/RtcscAppClientListenerInterface.h>
#include <RTCSCNativeInterface/RtcscAppClientInterface.h>
#include <RTCSCNativeInterface/Types/MediaConnectionState.h>
#include <RTCSCNativeInterface/Types/MediaSide.h>
#include <RTCSCNativeInterface/Types/MediaType.h>
#include <RTCSCNativeInterface/Types/Optional.h>
#include <RTCSCNativeInterface/Types/RtcscErrorCode.h>
#include <RTCSCNativeInterface/Types/SessionState.h>
#include <RTCSCNativeInterface/Types/VideoEffect.h>
#include <SmartScreenSDKInterfaces/DisplayCardState.h>
#include <SmartScreenSDKInterfaces/LiveViewControllerCapabilityAgentObserverInterface.h>

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace liveViewController {

/**
 * This class implements a @c CapabilityAgent that handles the Smart Home @c LiveViewController API.  The
 * @c LiveViewControllerCapabilityAgent is responsible for handling the directives with
 * Alexa.Camera.LiveViewController namespace.
 */
class LiveViewControllerCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public rtc::native::RtcscAppClientListenerInterface
        , public std::enable_shared_from_this<LiveViewControllerCapabilityAgent> {
public:
    /**
     * Create an instance of @c LiveViewControllerCapabilityAgent.
     *
     * @param focusManager The object to use to fetch visual focus for LiveViewController.
     * @param messageSender The object to use for sending events.
     * @param contextManager The AVS Context manager used to generate system context for events.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     * @return @c nullptr if the inputs are not defined, else a new instance of @c LiveViewControllerCapabilityAgent.
     */
    static std::shared_ptr<LiveViewControllerCapabilityAgent> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    /**
     * Destructor.
     */
    virtual ~LiveViewControllerCapabilityAgent() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    void onFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior) override;
    /// @}

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name RtcscAppClientListenerInterface Functions
    /// @{
    void onSessionAvailable(const std::string& sessionId) override;
    void onSessionRemoved(const std::string& sessionId) override;
    void onError(
        rtc::native::RtcscErrorCode errorCode,
        const std::string& errorMessage,
        const rtc::native::Optional<std::string>& sessionId) override;
    void onSessionStateChanged(const std::string& sessionId, rtc::native::SessionState sessionState) override;
    void onMediaStatusChanged(
        const std::string& sessionId,
        rtc::native::MediaSide mediaSide,
        rtc::native::MediaType mediaType,
        bool enabled) override;
    void onVideoEffectChanged(
        const std::string& sessionId,
        rtc::native::VideoEffect currentVideoEffect,
        int videoEffectDurationMs) override;
    void onMediaConnectionStateChanged(const std::string& sessionId, rtc::native::MediaConnectionState state) override;
    void onFirstFrameRendered(const std::string& sessionId, rtc::native::MediaSide mediaSide) override;
    void onFirstFrameReceived(const std::string& sessionId, rtc::native::MediaType mediaType) override;
    /// @}

    /**
     * Add an observer to the list of observers
     *
     * @param observer a shared pointer to a LiveViewControllerCapabilityAgentObserverInterface object
     */
    void addObserver(std::shared_ptr<
                     alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface>
                         observer);

    /**
     * Remove an observer from the list of observers
     *
     * @param observer a shared pointer to a LiveViewControllerCapabilityAgentObserverInterface object
     */
    void removeObserver(
        std::shared_ptr<
            alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface>
            observer);

    /**
     * Set the microphone state
     *
     * @param enabled whether the microphone should be turned on or not
     */
    void setMicrophoneState(bool enabled);

    /**
     * This function clears the live view from the screen and releases any focus being held.
     */
    void clearLiveView();

    /**
     * Set the executor used as the worker thread
     *
     * @param executor The @c Executor to set
     * @note This function should only be used for testing purposes. No call to any other method should
     * be done prior to this call.
     */
    void setExecutor(const std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor>& executor);

    /**
     * Set the rtcscAppClient used for communications
     *
     * @param rtcscAppClient The @c RtcscAppClient to set
     * @note This function should only be used for testing purposes. No call to any other method should
     * be done prior to this call.
     */
    void setRtcscAppClient(const std::shared_ptr<rtc::native::RtcscAppClientInterface>& rtcscAppClient);

private:
    /**
     * Constructor.
     *
     * @param focusManager The object to use to fetch visual focus for LiveViewController.
     * @param messageSender The object to use for sending events.
     * @param contextManager The AVS Context manager used to generate system context for events.
     * @param exceptionSender The object to use for sending AVS Exception messages.
     */
    LiveViewControllerCapabilityAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Creates the Alexa.Camera.LiveViewControllerCapabilityAgent interface configuration.
     *
     * @return The Alexa.Camera.LiveViewControllerCapabilityAgent interface configuration.
     */
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
    getLiveViewControllerCapabilityConfiguration();

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
     * This is a state machine function to handle the focus change event.
     *
     * @param newFocus the incoming @c FocusState.
     */
    void executeOnFocusChangedEvent(alexaClientSDK::avsCommon::avs::FocusState newFocus);

    /**
     * This is a state machine function to handle the clear live view event.
     */
    void executeClearLiveViewEvent();

    /**
     * This function handles a @c StartLiveView directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleStartLiveView(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c StopLiveView directive.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleStopLiveView(const std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by @c LiveViewController CA.
     *
     * @param info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This is a state machine function to handle the StartLiveView directive.
     */
    void executeStartLiveViewDirective(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     * This is a state machine function to handle the StopLiveView directive.
     */
    void executeStopLiveViewDirective(
        const std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> info);

    /**
     * This is an internal function that is called when the state machine is ready to notify the
     * @c LiveViewControllerCapabilityAgent observers to display live view.
     */
    void executeRenderLiveView();

    /**
     * This is an internal function that is called when the state machine is ready to notify the
     * @c LiveViewControllerCapabilityAgent observers to clear live view.
     */
    void executeClearLiveView();

    /**
     * This function handles the notification of the render live view callbacks to all the observers.  This function
     * is intended to be used in the context of @c m_executor worker thread.
     *
     * @param isClearLiveView whether to clear the current displayed live view or not.
     */
    void executeRenderLiveViewCallbacks(bool isClearLiveView);

    /**
     * This is an internal function that is called to notify the @c LiveViewControllerCapabilityAgent observers
     * camera state updates.
     *
     * @param cameraState the incoming camera state
     */
    void executeOnCameraStateChanged(smartScreenSDKInterfaces::CameraState cameraState);

    /**
     * This function deserializes a @c Directive's payload into a @c rapidjson::Document.
     *
     * @param info The @c DirectiveInfo to read the payload string from.
     * @param[out] document The @c rapidjson::Document to parse the payload into.
     * @return @c true if parsing was successful, else @c false.
     */
    bool parseDirectivePayload(std::shared_ptr<DirectiveInfo> info, rapidjson::Document* document);

    /**
     * This function sends a live view event back to the cloud
     *
     * @param eventName name of the live view event
     * @param payload payload of the live view event
     */
    void executeSendLiveViewEvent(const std::string& eventName, const std::string& payload);

    /**
     * If m_rtcscAppClient was previously set do nothing otherwise set a new instance of it
     */
    void executeInstantiateRtcscAppClient();

    /**
     * Internal function for disconnecting a given RTC client session.
     *
     * @param sessionId the id of the session to disconnect.
     * @param disconnectCode the reason for disconnect code.
     */
    void executeDisconnectRtcscSession(
        const std::string& sessionId,
        rtc::native::RtcscAppDisconnectCode disconnectCode);

    /**
     * Internal function for determining if there is an active @c StartLiveView directive.
     * @return true if there is an active @c StartLiveView directive.
     */
    bool hasActiveLiveView();

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// The object to use for sending events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// The @c ContextManager used to generate system context for events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The @c FocusManager used to manage usage of the visual channel.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    /// RTC App Client object
    std::shared_ptr<rtc::native::RtcscAppClientInterface> m_rtcscAppClient;

    /// The current focus state of the @c LiveViewController CA on the visual channel.
    alexaClientSDK::avsCommon::avs::FocusState m_focus;

    /// The state of the @c LiveViewController CA state machine.
    alexaSmartScreenSDK::smartScreenSDKInterfaces::State m_state;

    /// An unordered set of LiveViewControllerCapabilityAgent observers.
    std::unordered_set<std::shared_ptr<
        alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface>>
        m_observers;

    /// The directive corresponding to the last StartLiveView directive.
    std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityAgent::DirectiveInfo> m_lastDisplayedDirective;

    /// The sessionId obtained from the last StartLiveView directive.
    std::string m_lastSessionId;

    /// The concurrentTwoWayTalk obtained from the last StartLiveView directive.
    smartScreenSDKInterfaces::ConcurrentTwoWayTalk m_concurrentTwoWayTalk;

    /// The microphoneState obtained from the last StartLiveView directive.
    smartScreenSDKInterfaces::AudioState m_microphoneState;

    /// Interface that currently holds focus.
    std::string m_focusHoldingInterface;

    /// The AppInfo object.
    rtc::native::AppInfo m_appInfo;

    /// The target endpoint ID obtained from StartLiveView directive.
    std::string m_targetEndpointId;

    /// The target type obtained from StartLiveView directive.
    std::string m_targetType;

    /// This is the worker thread for the @c LiveViewControllerCapabilityAgent CA.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;
};

}  // namespace liveViewController
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_LIVEVIEWCONTROLLERCAPABILITYAGENT_INCLUDE_LIVEVIEWCONTROLLERCAPABILITYAGENT_LIVEVIEWCONTROLLERCAPABILITYAGENT_H_