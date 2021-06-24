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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREENGINELOGBRIDGE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREENGINELOGBRIDGE_H

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#undef DEBUG
#include <apl/utils/log.h>
#pragma pop_macro("DEBUG")
#pragma GCC diagnostic pop
#include "AplOptionsInterface.h"

namespace APLClient {
/// Wrapper around the Alexa Client SDK logger for use by APLCoreEngine
class AplCoreEngineLogBridge : public apl::LogBridge {
public:
    AplCoreEngineLogBridge(AplOptionsInterfacePtr aplOptions);

    void transport(apl::LogLevel level, const std::string& log) override;

private:
    AplOptionsInterfacePtr m_aplOptions;
};
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLCOREENGINELOGBRIDGE_H