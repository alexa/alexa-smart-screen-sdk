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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_DISPLAYCARDSTATE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_DISPLAYCARDSTATE_H_

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * This enum provides the visual focus state ownership of the capability agent.
 */
enum class State {
    /// The capability agent is idle.
    IDLE,

    /**
     * The capability agent has received a displayCard event is acquiring the visual channel from @c
     * FocusManager.
     */
    ACQUIRING,

    /**
     * The capability agent has focus, either background or foreground, of the channel and has
     * notified its observers of a displayCard.  The capability agent will remain in this state until there
     * is a timeout, clearCard, or focusChanged(NONE) event.
     */
    DISPLAYING,

    /**
     * The capability agent has received a timeout or a clearCard event and is releasing the
     * channel and has notified its observers to clear the display.
     */
    RELEASING,

    /**
     * The capability agent has received a displayCard event during releasing of the channel and is
     * trying to acquire the visual channel again.
     */
    REACQUIRING
};

/**
 * This is a function to convert the @c State to a string.
 */
inline std::string stateToString(const State state) {
    switch (state) {
        case State::IDLE:
            return "IDLE";
        case State::ACQUIRING:
            return "ACQUIRING";
        case State::DISPLAYING:
            return "DISPLAYING";
        case State::RELEASING:
            return "RELEASING";
        case State::REACQUIRING:
            return "REACQUIRING";
    }
    return "UNKNOWN";
}

/// Enum for the different non-player info types the agent may be displaying.
enum class NonPlayerInfoDisplayType {
    /// No non-player info display is presented.
    NONE,
    /// Presenting a RenderTemplate Card.
    RENDER_TEMPLATE,
    /// Presenting an APL Document.
    ALEXA_PRESENTATION
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_DISPLAYCARDSTATE_H_
