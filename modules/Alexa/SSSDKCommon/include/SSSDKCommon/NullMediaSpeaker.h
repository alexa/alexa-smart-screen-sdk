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

#ifndef ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIASPEAKER_H_
#define ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIASPEAKER_H_

#pragma once

#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

using namespace alexaClientSDK;

class NullMediaSpeaker : public avsCommon::sdkInterfaces::SpeakerInterface {
public:
    NullMediaSpeaker();
    /// @name SpeakerInterface methods.
    ///@{
    bool setVolume(int8_t volume) override;
    bool setMute(bool mute) override;
    bool getSpeakerSettings(SpeakerSettings* settings) override;
    ///@}
};
}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIASPEAKER_H_