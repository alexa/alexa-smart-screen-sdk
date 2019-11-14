/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_ACTIVITYEVENT_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_ACTIVITYEVENT_H_

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/// Enumeration of activity events that could be sent from GUI to @c AlexaPresentation.
enum class ActivityEvent {
    /// GUI switched to active state.
    ACTIVATED,

    /// GUI become inactive.
    DEACTIVATED,

    /// GUI processed one-time event (touch/scroll/etc).
    ONE_TIME,

    /// Interrupt event (touch)
    INTERRUPT,

    /// Guard option for unknown received state.
    UNKNOWN
};

/**
 * This function converts the provided string to an @c ActivityEvent.
 *
 * @param string The string to convert to @c ActivityEvent.
 * @return The @c ActivityEvent.
 */
inline ActivityEvent activityEventFromString(const std::string& string) {
    if ("ACTIVATED" == string) {
        return ActivityEvent::ACTIVATED;
    } else if ("DEACTIVATED" == string) {
        return ActivityEvent::DEACTIVATED;
    } else if ("ONE_TIME" == string) {
        return ActivityEvent::ONE_TIME;
    } else if ("INTERRUPT" == string) {
        return ActivityEvent::INTERRUPT;
    } else {
        return ActivityEvent::UNKNOWN;
    }
}

/**
 * This function converts an ActivityEvent to a string.
 *
 * @param event the ActivityEvent to be converted.
 * @return the string for the input ActivityEvent.
 */
inline std::string activityEventToString(ActivityEvent event) {
    switch (event) {
        case ActivityEvent::ACTIVATED:
            return "ACTIVATED";
        case ActivityEvent::DEACTIVATED:
            return "DEACTIVATED";
        case ActivityEvent::ONE_TIME:
            return "ONE_TIME";
        case ActivityEvent::INTERRUPT:
            return "INTERRUPT";
        default:
            return "UNKNOWN";
    }
}

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_ACTIVITYEVENT_H_