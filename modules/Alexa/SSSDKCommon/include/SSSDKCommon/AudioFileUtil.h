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

#ifndef ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_AUDIOFILEUTIL_H_
#define ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_AUDIOFILEUTIL_H_

#pragma once

#include <vector>
#include <string>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

class AudioFileUtil {
public:
    static std::vector<int16_t> readAudioFromFile(const std::string& fileName, bool& errorOccurred);
};

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_AUDIOFILEUTIL_H_
