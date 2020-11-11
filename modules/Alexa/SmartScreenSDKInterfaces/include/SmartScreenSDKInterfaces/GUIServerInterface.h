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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_

#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>

#include <APLClient/AplRenderingEvent.h>
#include <APLClient/DisplayMetric.h>
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
     * Toggles the microphone state if the Sample App was built with wake word. When the microphone is turned off, the
     * app enters a privacy mode in which it stops recording audio data from the microphone, thus disabling Alexa waking
     * up due to wake word. Note however that hold-to-talk and tap-to-talk modes will still work by recording
     * microphone data temporarily until a user initiated interaction is complete. If this app was built without wake
     * word then this will do nothing as the microphone is already off.
     */
    virtual void handleMicrophoneToggle() = 0;

    /**
     * Handle playback 'PLAY' event.
     */
    virtual void handlePlaybackPlay() = 0;

    /**
     * Handle playback 'PAUSE' event.
     */
    virtual void handlePlaybackPause() = 0;

    /**
     * Handle playback 'NEXT' event.
     */
    virtual void handlePlaybackNext() = 0;

    /**
     * Handle playback 'PREVIOUS' event.
     */
    virtual void handlePlaybackPrevious() = 0;

    /**
     * Handle playback 'SKIP_FORWARD' event.
     */
    virtual void handlePlaybackSkipForward() = 0;

    /**
     * Handle playback 'SKIP_BACKWARD' event.
     */
    virtual void handlePlaybackSkipBackward() = 0;

    /**
     * Handle playback 'TOGGLE' event.
     * @param name name of the TOGGLE.
     * @param checked checked state of the TOGGLE.
     */
    virtual void handlePlaybackToggle(const std::string& name, bool checked) = 0;

    /**
     * Handle userEvent by parsing out payload from message.
     *
     * @param userEventPayload the user event payload.
     */
    virtual void handleUserEvent(std::string userEventPayload) = 0;

    /**
     * Handle DataSourceFetchRequestEvent.
     *
     * @param type type of DataSource asking for fetch.
     * @param payload event payload.
     */
    virtual void handleDataSourceFetchRequestEvent(std::string type, std::string payload) = 0;

    /**
     * Handle RuntimeError event.
     *
     * @param payload event payload.
     */
    virtual void handleRuntimeErrorEvent(std::string payload) = 0;

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

    /**
     * Gets Device Time Zone Offset.
     */
    virtual std::chrono::milliseconds getDeviceTimezoneOffset() = 0;

    /**
     * Gets Active AudioItem Audio Offset.
     */
    virtual std::chrono::milliseconds getAudioItemOffset() = 0;

    virtual void onUserEvent() = 0;

    /**
     * Force exit to reset focus state and clear card.
     */
    virtual void forceExit() = 0;

    /**
     * Handle render complete event received in a message.
     */
    virtual void handleRenderComplete() = 0;

    /**
     * Handle display metrics event received in a message.
     *
     * @param dropFrameCount Count of the frames dropped
     */
    virtual void handleDisplayMetrics(uint64_t dropFrameCount) = 0;

    /**
     * Handle display metrics event received in a message.
     *
     * @param metrics the recorded metrics
     */
    virtual void handleDisplayMetrics(const std::vector<APLClient::DisplayMetric> &metrics) = 0;

    /**
     * Handle APL context inflate started event.
     *
     * @param event The event in context
     */
    virtual void handleAPLEvent(APLClient::AplRenderingEvent event) = 0;

    /**
     * Handle toggle Do Not Disturb event
     */
    virtual void handleToggleDoNotDisturbEvent() = 0;

    /**
     * Handle an onConnectionOpened event from the messaging server
     */
    virtual void handleOnMessagingServerConnectionOpened() = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUISERVERINTERFACE_H_
