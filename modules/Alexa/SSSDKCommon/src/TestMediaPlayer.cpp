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

#include <atomic>

#include "SSSDKCommon/TestMediaPlayer.h"

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

using namespace alexaClientSDK;

/// String to identify log entries originating from this file.
static const std::string TAG("TestMediaPlayer");

/// A counter used to increment the source id when a new source is set.
static std::atomic<avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId> g_sourceId{0};

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

TestMediaPlayer::TestMediaPlayer() : RequiresShutdown("NullMediaPlayer") {
}

TestMediaPlayer::~TestMediaPlayer() {
}

void TestMediaPlayer::setEqualizerBandLevels(acsdkEqualizerInterfaces::EqualizerBandLevelMap bandLevelMap) {
}

int TestMediaPlayer::getMinimumBandLevel() {
    return 0;
}

int TestMediaPlayer::getMaximumBandLevel() {
    return 0;
}

avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId TestMediaPlayer::setSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    m_attachmentReader = std::move(attachmentReader);
    return ++g_sourceId;
}

avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId TestMediaPlayer::setSource(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader> attachmentReader,
    std::chrono::milliseconds offsetAdjustment,
    const alexaClientSDK::avsCommon::utils::AudioFormat* format,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config) {
    m_attachmentReader = std::move(attachmentReader);
    return ++g_sourceId;
}

alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId TestMediaPlayer::setSource(
    std::shared_ptr<std::istream> stream,
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    alexaClientSDK::avsCommon::utils::MediaType format) {
    return ++g_sourceId;
}

avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId TestMediaPlayer::setSource(
    const std::string& url,
    std::chrono::milliseconds offset,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::SourceConfig& config,
    bool repeat,
    const alexaClientSDK::avsCommon::utils::mediaPlayer::PlaybackContext& playbackContext) {
    return ++g_sourceId;
}

bool TestMediaPlayer::play(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    if (m_observer) {
        m_observer->onPlaybackStarted(id, avsCommon::utils::mediaPlayer::MediaPlayerState());
        m_playbackFinished = true;
        m_timer = std::unique_ptr<avsCommon::utils::timing::Timer>(new avsCommon::utils::timing::Timer);
        // Wait 600 milliseconds before sending onPlaybackFinished.
        m_timer->start(std::chrono::milliseconds(3000), [this, id] {
            if (m_playbackFinished) {
                m_observer->onPlaybackFinished(id, avsCommon::utils::mediaPlayer::MediaPlayerState());
                m_playbackFinished = false;
            }
        });
        return true;
    } else {
        return false;
    }
}

bool TestMediaPlayer::stop(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    if (m_observer && m_playbackFinished) {
        m_observer->onPlaybackStopped(id, avsCommon::utils::mediaPlayer::MediaPlayerState());
        m_playbackFinished = false;
        return true;
    } else {
        return false;
    }
}

// TODO Add implementation
bool TestMediaPlayer::pause(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    return true;
}

// TODO Add implementation
bool TestMediaPlayer::resume(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    return true;
}

std::chrono::milliseconds TestMediaPlayer::getOffset(avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    return std::chrono::milliseconds::zero();
}

uint64_t TestMediaPlayer::getNumBytesBuffered() {
    return 0;
}

void TestMediaPlayer::doShutdown() {
}

alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState>
TestMediaPlayer::getMediaPlayerState(alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerInterface::SourceId id) {
    return alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerState();
}

void TestMediaPlayer::addObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> playerObserver) {
    // TODO keep a list of observers
    m_observer = playerObserver;
}

void TestMediaPlayer::removeObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::mediaPlayer::MediaPlayerObserverInterface> playerObserver) {
    // do nothing
}

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK
