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

#include "AlexaPresentation/TimeoutType.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {

static const std::string SHORT_STR = "SHORT";
static const std::chrono::milliseconds SHORT_TIME(30000);

static const std::string TRANSIENT_STR = "TRANSIENT";
static const std::chrono::milliseconds TRANSIENT_TIME(10000);

static const std::string LONG_STR = "LONG";

using namespace alexaClientSDK::avsCommon::utils;

/// https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/presentation-apl.html#renderdocument
Optional<TimeoutType> TimeoutTypeUtils::fromString(const std::string& timeoutType) {
    alexaClientSDK::avsCommon::utils::Optional<TimeoutType> result;

    if (SHORT_STR == timeoutType) {
        result.set(TimeoutType::SHORT);
    } else if (TRANSIENT_STR == timeoutType) {
        result.set(TimeoutType::TRANSIENT);
    } else if (LONG_STR == timeoutType) {
        result.set(TimeoutType::LONG);
    }

    return result;
}

Optional<std::chrono::milliseconds> TimeoutTypeUtils::asDuration(TimeoutType timeoutType) {
    Optional<std::chrono::milliseconds> result{SHORT_TIME};
    switch (timeoutType) {
        case TimeoutType::SHORT:
            // initialized to SHORT_TIME
            break;
        case TimeoutType::TRANSIENT:
            result = TRANSIENT_TIME;
            break;
        case TimeoutType::LONG:
            result.reset();
            break;
            // note: not including default clause here, since we want to highlight unhandled cases explicitly
    }

    return result;
}

}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
