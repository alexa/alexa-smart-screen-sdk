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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_TIMEOUTTYPE_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_TIMEOUTTYPE_H_

#include <chrono>

#include <AVSCommon/Utils/Optional.h>

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

/**
 * Strongly-typed timeoutType as defined in the API specification
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#renderdocument
 */
enum class TimeoutType { SHORT, TRANSIENT, LONG };

/**
 * TimeoutType conversion utilities
 */
class TimeoutTypeUtils {
public:
    /**
     * Converts a string representation of timeoutType to strongly-typed enum
     *
     * @param timeoutType a string value
     * @return a optional value that contains TimeoutType or nothing if the input is invalid
     */
    static alexaClientSDK::avsCommon::utils::Optional<TimeoutType> fromString(const std::string& timeoutType);

    /**
     * Converts a TimeoutType value to a duration value
     * @param timeoutType a timeoutType value
     * @return duration in milliseconds
     */
    static alexaClientSDK::avsCommon::utils::Optional<std::chrono::milliseconds> asDuration(TimeoutType timeoutType);
};

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_ALEXAPRESENTATION_INCLUDE_ALEXAPRESENTATION_TIMEOUTTYPE_H_
