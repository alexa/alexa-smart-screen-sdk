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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_

#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>

#include "ActivityEvent.h"
#include "NavigationEvent.h"

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/// An interface providing APIs to be used by GUIClient.
class GUIServerInterface {
public:
    /**
     * Handle TapToTalk event.
     */
    virtual void handleTapToTalk() = 0;

    /**
     * Handle HoldToTalk event.
     */
    virtual void handleHoldToTalk() = 0;

    /**
     * Handle the MicrophoneToggle event.
     */
    virtual void handleMicrophoneToggle() = 0;

    /**
     * Handle userEvent by parsing out payload from message.
     *
     * @param userEventPayload the user event payload.
     */
    virtual void handleUserEvent(std::string userEventPayload) = 0;

    /**
     * Handle visual context received in a message.
     *
     * @param token The token visual context token.
     * @param payload The visual context payload.
     */
    virtual void handleVisualContext(uint64_t token, std::string payload) = 0;

    /**
     * Handle focus acquire requests.
     *
     * @param channelName channelName to be requested.
     * @param channelObserver the channelObserver to be notified.
     */
    virtual bool handleFocusAcquireRequest(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) = 0;

    /**
     * Handle focus release requests.
     *
     * @param channelName channelName to be released.
     * @param channelObserver the channelObserver to be notified.
     */
    virtual bool handleFocusReleaseRequest(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) = 0;

    /**
     * Handle RenderDocument result message.
     *
     * @param token the render document result token.
     * @param result the render document result.
     * @param error the render document error message.
     */
    virtual void handleRenderDocumentResult(std::string token, bool result, std::string error) = 0;

    /**
     * Handle ExecuteCommands result message.
     *
     * @param token the execute command result token.
     * @param result the execute command result.
     * @param error the execute command error message.
     */
    virtual void handleExecuteCommandsResult(std::string token, bool result, std::string error) = 0;

    /**
     * Handle activityEvent message.
     *
     * @param event the activity event.
     */
    virtual void handleActivityEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event) = 0;

    /**
     * Handle activityEvent message.
     *
     * @param source The source of the activity event
     * @param event the activity event.
     */
    virtual void handleActivityEvent(
        const std::string& source,
        alexaSmartScreenSDK::smartScreenSDKInterfaces::ActivityEvent event) = 0;

    /**
     * Handle event the navigation event.
     *
     * @param event the navigation event.
     */
    virtual void handleNavigationEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::NavigationEvent event) = 0;

    /**
     * Set custom document timeout. Will be reset for every directive received.
     *
     * @param timeout timeout in milliseconds.
     */
    virtual void setDocumentIdleTimeout(std::chrono::milliseconds timeout) = 0;

    /**
     * Handle device window state received in a message.
     *
     * @param payload The device window state payload.
     */
    virtual void handleDeviceWindowState(std::string payload) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_
