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

#include "APLClient/AplCoreEngineLogBridge.h"

static const std::string TAG("AplCoreEngine");

namespace APLClient {

AplCoreEngineLogBridge::AplCoreEngineLogBridge(AplOptionsInterfacePtr aplOptions) : m_aplOptions{aplOptions} {
}

void AplCoreEngineLogBridge::transport(apl::LogLevel level, const std::string& log) {
    switch (level) {
        case apl::LogLevel::TRACE:
            m_aplOptions->logMessage(LogLevel::TRACE, TAG, log);
            break;
            // TODO: Same problem as in AplCoreGuiRenderer.h but not solved by undef by some reason.
        case static_cast<apl::LogLevel>(1):
            m_aplOptions->logMessage(LogLevel::DBG, TAG, log);
            break;
        case apl::LogLevel::INFO:
            m_aplOptions->logMessage(LogLevel::INFO, TAG, log);
            break;
        case apl::LogLevel::WARN:
            m_aplOptions->logMessage(LogLevel::WARN, TAG, log);
            break;
        case apl::LogLevel::ERROR:
            m_aplOptions->logMessage(LogLevel::ERROR, TAG, log);
            break;
        case apl::LogLevel::CRITICAL:
            m_aplOptions->logMessage(LogLevel::CRITICAL, TAG, log);
            break;
        default:
            m_aplOptions->logMessage(LogLevel::ERROR, "AplCoreEngineUnknownLogLevel", log);
            return;
    }
}
}  // namespace APLClient