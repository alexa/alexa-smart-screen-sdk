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

#ifndef ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMICROPHONE_H_
#define ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMICROPHONE_H_

#pragma once
#include <Audio/MicrophoneInterface.h>
#include <AVSCommon/AVS/AudioInputStream.h>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

class NullMicrophone : public alexaClientSDK::applicationUtilities::resources::audio::MicrophoneInterface {
public:
    using SharedDataStream = alexaClientSDK::avsCommon::avs::AudioInputStream;
    using BufferWriter = alexaClientSDK::avsCommon::avs::AudioInputStream::Writer;

    NullMicrophone(std::shared_ptr<SharedDataStream> sharedDataStream);
    bool stopStreamingMicrophoneData() override;
    bool startStreamingMicrophoneData() override;
    void writeAudioData(const std::vector<int16_t>& data);
    bool isStreaming() override;

private:
    std::shared_ptr<SharedDataStream> m_sharedDataStream;
};

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMICROPHONE_H_
