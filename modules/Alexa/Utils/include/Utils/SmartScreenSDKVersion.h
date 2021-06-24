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


#ifndef ALEXA_SMART_SCREEN_SDK_UTILS_INCLUDE_UTILS_SMARTSCREENSDKVERSION_H_
#define ALEXA_SMART_SCREEN_SDK_UTILS_INCLUDE_UTILS_SMARTSCREENSDKVERSION_H_

#include <string>

namespace alexaSmartScreenSDK {
namespace utils {
/// These functions are responsible for providing access to the current Smart Screen SDK version.
/// NOTE: To make changes to this file you *MUST* do so via SmartScreenSDKVersion.h.in.
namespace smartScreenSDKVersion{
    inline static std::string getCurrentVersion(){
        return "2.7";
    }
    inline static int getMajorVersion(){
        return 2;
    }
    inline static int getMinorVersion(){
        return 7;
    }
}  // namespace smartScreenSDKVersion
}  // namespace utils
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_UTILS_INCLUDE_UTILS_SMARTSCREENSDKVERSION_H_
