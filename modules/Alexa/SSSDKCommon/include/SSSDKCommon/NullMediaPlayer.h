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

#ifndef ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIAPLAYER_H_
#define ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIAPLAYER_H_

#pragma once

#include <acsdkEqualizerInterfaces/EqualizerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

using namespace alexaClientSDK;
using SourceId = avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId;

class NullMediaPlayer
        : public avsCommon::utils::mediaPlayer::MediaPlayerInterface
        , public acsdkEqualizerInterfaces::EqualizerInterface
        , public avsCommon::utils::RequiresShutdown {
public:
    NullMediaPlayer();

    /// @name EqualizerInterface methods.
    ///@{
    void setEqualizerBandLevels(acsdkEqualizerInterfaces::EqualizerBandLevelMap bandLevelMap) override;

    int getMinimumBandLevel() override;

    int getMaximumBandLevel() override;
    ///}@

    /// @name MediaPlayerInterface methods.
    ///@{
    SourceId setSource(
        std::shared_ptr<avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const avsCommon::utils::AudioFormat* format) override;

    SourceId setSource(const std::string& url, std::chrono::milliseconds offset, bool repeat) override;

    SourceId setSource(std::shared_ptr<std::istream> stream, bool repeat) override;

    bool play(SourceId id) override;

    bool stop(SourceId id) override;

    bool pause(SourceId id) override;

    bool resume(SourceId id) override;

    std::chrono::milliseconds getOffset(SourceId id) override;

    uint64_t getNumBytesBuffered() override;

    void setObserver(
        std::shared_ptr<avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> playerObserver) override;
    ///@}

protected:
    /// @name RequiresShutdown methods.
    /// @{
    void doShutdown() override;
    /// @}
};
}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_NULLMEDIAPLAYER_H_
