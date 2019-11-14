/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FOCUSBRIDGE_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FOCUSBRIDGE_H_

#include <map>
#include <memory>
#include <mutex>

#include <AVSCommon/AVS/FocusState.h>
#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include <SmartScreenSDKInterfaces/MessagingInterface.h>
#include <SmartScreenClient/SmartScreenClient.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * A FocusBridge takes care of routing acquire and release channel requests from GUI (APL) requestors to appropriate
 * FocusManager. It also routes channel state changes.
 */
class FocusBridge
        : public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<FocusBridge> {
public:
    /// Alias for GUI provided token.
    using APLToken = uint64_t;

    /**
     * Constructor.
     */
    FocusBridge(
        std::shared_ptr<smartScreenClient::SmartScreenClient> client,
        std::shared_ptr<smartScreenSDKInterfaces::MessagingInterface> messagingInterface);

    /**
     * Process FocusManager focus acquire request from APL.
     *
     * @param token Operation token - unique per acquire/release request pair.
     * @param channelName Name of channel to acquire.
     * @param avsInterface AVS interface to report as owner of channel.
     */
    void processFocusAcquireRequest(
        const APLToken token,
        const std::string& channelName,
        const std::string& avsInterface);

    /**
     * Process FocusManager focus release request from APL.
     *
     * @param token Operation token - unique per acquire/release request pair.
     * @param channelName Name of channel to release.
     */
    void processFocusReleaseRequest(const APLToken token, const std::string& channelName);

    /**
     * Process confirmation from APL that "onFocusChanged" was received.
     *
     * @param token Operation token.
     */
    void processOnFocusChangedReceivedConfirmation(const APLToken token);

private:
    /**
     * This class represents requestors as clients of FocusManager and handle notifications.
     */
    class ProxyFocusObserver : public alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface {
    public:
        /**
         * Constructor.
         *
         * @param token Requester token.
         * @param focusBridge FocusBridge to perform operations through.
         * @param channelName Name of related channel.
         */
        ProxyFocusObserver(
            const APLToken token,
            std::shared_ptr<FocusBridge> focusBridge,
            const std::string& channelName);

        /// @name ChannelObserverInterface Functions
        /// @{
        void onFocusChanged(alexaClientSDK::avsCommon::avs::FocusState newFocus) override;
        ///@}

    private:
        /// Related requester token.
        APLToken m_token;

        /// Parent FocusBridge.
        std::shared_ptr<FocusBridge> m_focusBridge;

        /// Focus channel name.
        const std::string m_channelName;
    };

    /**
     * Handle autoRelease.
     * @param token Requester token.
     * @param channelName Channel name.
     */
    void autoRelease(const APLToken token, const std::string& channelName);

    /**
     * Start auto release timer.
     *
     * @param token Requester token.
     * @param channelName Channel name.
     */
    void startAutoreleaseTimer(const APLToken token, const std::string& channelName);

    /**
     * Send focus response.
     *
     * @param token Requester token.
     * @param result Result of focus operation.
     */
    void sendFocusResponse(const APLToken token, const bool result);

    /**
     * Send focus change event notification.
     *
     * @param token Requester token.
     * @param state Resulting channel state.
     */
    void sendOnFocusChanged(const APLToken token, const alexaClientSDK::avsCommon::avs::FocusState state);

    /// Internal function to execute @see processFocusAcquireRequest
    void executeFocusAcquireRequest(
        const APLToken token,
        const std::string& channelName,
        const std::string& avsInterface);

    /// Internal function to execute @see processFocusReleaseRequest
    void executeFocusReleaseRequest(const APLToken token, const std::string& channelName);

    /**
     * Helper to get appropriate FocusManager for requested channel.
     *
     * @param channelName Channel name.
     * @return
     */
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getFocusManagerInterfaceForChannel(
        const std::string& channelName);

    /// @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /// The default SDK client.
    std::shared_ptr<smartScreenClient::SmartScreenClient> m_client;

    /// A reference to the generic messaging interface that JSON messages are forwarded to.
    std::shared_ptr<smartScreenSDKInterfaces::MessagingInterface> m_messagingInterface;

    /// A map of APL side focus observers (proxies).
    std::map<APLToken, std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface>>
        m_focusObservers;

    /// Autorelease timers for case when client not received channel state change message.
    std::map<APLToken, std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::Timer>> m_autoReleaseTimers;

    /// Mutex for requester maps.
    std::mutex m_mapMutex;

    /// An internal executor that performs execution of callable objects passed to it sequentially but asynchronously.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_FOCUSBRIDGE_H_
