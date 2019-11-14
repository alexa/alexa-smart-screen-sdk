/*
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_GUILOGBRIDGE_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_GUILOGBRIDGE_H

#include <string>

#include <AVSCommon/Utils/Threading/Executor.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * Simple class to direct GUI log to SDK log.
 */
class GUILogBridge {
public:
    /**
     * Transform GUI log event level to SDK one according to guidelines and log it.
     *
     * @param level Log level. One of "trace", "debug", "info", "warn", "error".
     * @param component Renderer internal component that produced event.
     * @param message Event/log message.
     */
    void log(const std::string& level, const std::string& component, const std::string& message);

private:
    /**
     * Internal function to execute logging.
     */
    void executeLog(const std::string& level, const std::string& component, const std::string& message);

    /// This is the worker thread for the @c GUILogBridge.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_GUILOGBRIDGE_H
