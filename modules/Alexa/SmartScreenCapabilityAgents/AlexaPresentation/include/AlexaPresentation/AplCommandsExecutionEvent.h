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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_APLCOMMANDEXECUTIONEVENT_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_APLCOMMANDEXECUTIONEVENT_H_

#include <chrono>

#include <AVSCommon/Utils/Optional.h>

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

static const std::string RESOLVED_STR = "RESOLVED";
static const std::string TERMINATED_STR = "TERMINATED";
static const std::string FAILED_STR = "FAILED";

/// Enumeration of APL Command Execution Events that can be reported to the agent by the runtime.
enum class AplCommandExecutionEvent {
    /// APL Runtime resolved the commands sequence.
    RESOLVED,

    /// APL Runtime terminated the commands sequence.
    TERMINATED,

    /// APL Runtime failed to parse/handle the commands sequence.
    FAILED
};

/**
 * This is a function to convert the @c AplCommandExecutionEvent to a string.
 */
inline std::string commandExecutionEventToString(const AplCommandExecutionEvent event) {
    switch (event) {
        case AplCommandExecutionEvent::RESOLVED:
            return "RESOLVED";
        case AplCommandExecutionEvent::TERMINATED:
            return "TERMINATED";
        case AplCommandExecutionEvent::FAILED:
            return "FAILED";
    }
    return "UNKNOWN";
}

/**
 * This is a function to convert a string to a @c AplCommandExecutionEvent.
 */
inline AplCommandExecutionEvent stringToCommandExecutionEvent(const std::string& eventStr) {
    if (RESOLVED_STR == eventStr) {
        return AplCommandExecutionEvent::RESOLVED;
    } else if (TERMINATED_STR == eventStr) {
        return AplCommandExecutionEvent::TERMINATED;
    } else if (FAILED_STR == eventStr) {
        return AplCommandExecutionEvent::FAILED;
    }

    return AplCommandExecutionEvent::FAILED;
}

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_APLCOMMANDEXECUTIONEVENT_H_
