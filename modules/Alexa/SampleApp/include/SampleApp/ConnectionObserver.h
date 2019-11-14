/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CONNECTIONOBSERVER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CONNECTIONOBSERVER_H_

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <AVSCommon/SDKInterfaces/AuthObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * A class that observes the status of authorization and connection to AVS.
 */
class ConnectionObserver
        : public alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface {
public:
    /**
     * Constructor.
     */
    ConnectionObserver();

    void onAuthStateChange(
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State newState,
        alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error error) override;

    void onConnectionStatusChanged(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason)
        override;

    /**
     * Waits for the specified authorization state.
     *
     * @param authState The auth state to wait for.
     * @param duration The optional duration to wait for. This defaults to 20 seconds.
     * @return Whether the state was successfully reached.
     */
    bool waitFor(
        const alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State authState,
        const std::chrono::seconds duration = std::chrono::seconds(20));

    /**
     * Waits for the specified connection state.
     *
     * @param authState The connection state to wait for.
     * @param duration The optional duration to wait for. This defaults to 20 seconds.
     * @return Whether the state was successfully reached.
     */
    bool waitFor(
        const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status connectionStatus,
        const std::chrono::seconds duration = std::chrono::seconds(20));

private:
    /// Internal mutex to serialize access to m_connectionStatus and m_authState states.
    std::mutex m_mutex;

    /// A condition variable used to wait for state changes.
    std::condition_variable m_trigger;

    /// The current authorization state.
    alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State m_authState;

    /// The current connection state.
    alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status m_connectionStatus;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_CONNECTIONOBSERVER_H_
