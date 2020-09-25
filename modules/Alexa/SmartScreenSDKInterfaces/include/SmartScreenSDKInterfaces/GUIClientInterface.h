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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUICLIENTINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUICLIENTINTERFACE_H

#include <AVSCommon/AVS/FocusState.h>
#include <AVSCommon/SDKInterfaces/ChannelObserverInterface.h>

#include "AlexaPresentationObserverInterface.h"
#include "AudioPlayerInfo.h"
#include "GUIServerInterface.h"
#include "MessageInterface.h"
#include "TemplateRuntimeObserverInterface.h"
#include "VisualStateProviderInterface.h"

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * An interface representing the GUI component responsible for rendering display card and for APL.
 */
// TODO: Elaborate the requirements for an implementation of GUIClientInterface (ARC-905)
class GUIClientInterface
        : public virtual AlexaPresentationObserverInterface
        , public virtual TemplateRuntimeObserverInterface
        , public virtual VisualStateProviderInterface {
public:
    /**
     * Destructor
     */
    virtual ~GUIClientInterface() = default;

    /**
     * Set a reference to a GUI Manager
     * @param guiManager Client related operations.
     */
    virtual void setGUIManager(
        std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface> guiManager) = 0;

    /**
     * Request audio focus.
     * @param channelName The channel to be requested.
     * @param channelObserver the channelObserver to be notified.
     */
    virtual bool acquireFocus(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) = 0;

    /**
     * Release audio focus.
     * @param channelName The channel to be released.
     * @param channelObserver the channelObserver to be notified.
     */
    virtual bool releaseFocus(
        std::string channelName,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) = 0;

    /**
     * Sends a GUI Message to the server.
     * @param message The message to be wrriten.
     */
    virtual void sendMessage(MessageInterface& message) = 0;

    /**
     * Handle an @c NavigationEvent
     * @param event The @c NavigationEvent to handle.
     * @return True if the event was successfully handled by the client.
     */
    virtual bool handleNavigationEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::NavigationEvent event) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_GUICLIENTINTERFACE_H
