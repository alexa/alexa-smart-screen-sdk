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

#include <AVSCommon/Utils/Logger/Logger.h>
#include "SampleApp/GUILogBridge.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

/// String to identify log entries originating from this file.
static const std::string TAG("GUILogBridge");

/// String to identify event happened.
static const std::string GUI_LOG_EVENT("GUILog");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

void GUILogBridge::log(const std::string& level, const std::string& component, const std::string& message) {
    m_executor.submit([this, level, component, message]() { executeLog(level, component, message); });
}

void GUILogBridge::executeLog(const std::string& level, const std::string& component, const std::string& message) {
    if ("trace" == level) {
        ACSDK_DEBUG9(LX(GUI_LOG_EVENT).d("component", component).m(message));
    } else if ("debug" == level) {
        ACSDK_DEBUG5(LX(GUI_LOG_EVENT).d("component", component).m(message));
    } else if ("info" == level) {
        ACSDK_DEBUG3(LX(GUI_LOG_EVENT).d("component", component).m(message));
    } else if ("warn" == level) {
        ACSDK_WARN(LX(GUI_LOG_EVENT).d("component", component).m(message));
    } else if ("error" == level) {
        ACSDK_ERROR(LX(GUI_LOG_EVENT).d("component", component).m(message));
    } else {
        ACSDK_ERROR(LX("logFailed").d("reason", "Unsupported log level.").d("component", component).d("level", level));
    }
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK