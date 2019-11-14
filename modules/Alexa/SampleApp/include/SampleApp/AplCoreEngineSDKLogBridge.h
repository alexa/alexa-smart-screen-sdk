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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCOREENGINESDKLOGBRIDGE_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCOREENGINESDKLOGBRIDGE_H

#include <string>

// TODO: Tidy up core to prevent this (ARC-917)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#undef DEBUG
#include <apl/utils/log.h>
#pragma pop_macro("DEBUG")
#pragma GCC diagnostic pop

namespace alexaSmartScreenSDK {
namespace sampleApp {
/// Wrapper around the Alexa Client SDK logger for use by APLCoreEngine
class AplCoreEngineSDKLogBridge : public apl::LogBridge {
public:
    void transport(apl::LogLevel level, const std::string& log) override;
};
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_APLCOREENGINESDKLOGBRIDGE_H