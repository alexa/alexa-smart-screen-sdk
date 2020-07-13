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

#ifndef ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_TESTMEDIAPLAYER_H_
#define ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_TESTMEDIAPLAYER_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <iostream>
#include <string>
#include <future>

#include <AVSCommon/Utils/MediaPlayer/MediaPlayerInterface.h>
#include <AVSCommon/Utils/MediaPlayer/MediaPlayerObserverInterface.h>
#include <AVSCommon/Utils/MediaPlayer/SourceConfig.h>
#include "AVSCommon/Utils/Timing/Timer.h"
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/SDKInterfaces/Audio/EqualizerInterface.h>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

/**
 * A Mock MediaPlayer that attempts to alert the observer of playing and stopping without
 * actually playing audio. This removes the dependancy on an audio player to run tests with
 * SpeechSynthesizer
 */
class TestMediaPlayer
        : public alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public alexaClientSDK::avsCommon::sdkInterfaces::audio::EqualizerInterface {
public:
    TestMediaPlayer();
    // Destructor.
    ~TestMediaPlayer();

    /// @name EqualizerInterface methods.
    ///@{
    void setEqualizerBandLevels(
        alexaClientSDK::avsCommon::sdkInterfaces::audio::EqualizerBandLevelMap bandLevelMap) override;

    int getMinimumBandLevel() override;

    int getMaximumBandLevel() override;
    ///}@

    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
        const alexaClientSDK::avsCommon::utils::AudioFormat* format,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config =
            alexaClientSDK::avsCommon::utils::mediaPlayer::emptySourceConfig()) override;

    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        std::shared_ptr<std::istream> stream,
        bool repeat,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config =
            alexaClientSDK::avsCommon::utils::mediaPlayer::emptySourceConfig(),
        alexaClientSDK::avsCommon::utils::MediaType format =
            alexaClientSDK::avsCommon::utils::MediaType::UNKNOWN) override;

    alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId setSource(
        const std::string& url,
        std::chrono::milliseconds offset = std::chrono::milliseconds::zero(),
        const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config =
            alexaClientSDK::avsCommon::utils::mediaPlayer::emptySourceConfig(),
        bool repeat = false,
        const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext =
            alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext()) override;

    bool play(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    bool stop(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    bool pause(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    bool resume(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    std::chrono::milliseconds getOffset(
        alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    uint64_t getNumBytesBuffered() override;

    alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
    getMediaPlayerState(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) override;

    void addObserver(std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface>
                         playerObserver) override;

    void removeObserver(std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface>
                            playerObserver) override;

protected:
    /// @name RequiresShutdown methods.
    /// @{
    void doShutdown() override;
    /// @}

private:
    /// Observer to notify of state changes.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> m_observer;
    /// Flag to indicate when a playback finished notification has been sent to the observer.
    bool m_playbackFinished = false;
    /// The AttachmentReader to read audioData from.
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> m_attachmentReader;
    /// Timer to wait to send onPlaybackFinished to the observer.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::Timer> m_timer;
    // istream for Alerts.
    std::shared_ptr<std::istream> m_istream;
};

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SSSDKCOMMON_INCLUDE_SSSDKCOMMON_TESTMEDIAPLAYER_H_
