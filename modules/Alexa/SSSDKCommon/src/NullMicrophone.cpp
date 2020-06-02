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

#include <fstream>

#include "SSSDKCommon/NullMicrophone.h"

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

using namespace alexaClientSDK;

NullMicrophone::NullMicrophone(std::shared_ptr<SharedDataStream> sharedDataStream) :
        m_sharedDataStream{sharedDataStream} {
}

bool NullMicrophone::stopStreamingMicrophoneData() {
    return true;
}

bool NullMicrophone::startStreamingMicrophoneData() {
    return true;
}

void NullMicrophone::writeAudioData(const std::vector<int16_t>& data) {
    auto audioBufferWriter =
        m_sharedDataStream->createWriter(avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);
    audioBufferWriter->write(data.data(), data.size());
}

bool NullMicrophone::isStreaming() {
    return true;
}

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK
