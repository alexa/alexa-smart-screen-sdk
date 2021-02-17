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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSIONOBSERVERINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSIONOBSERVERINTERFACE_H

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/**
 * This class allows a @c AplAudioPlayerExtensionObserverInterface observer to be notified of changes in the
 * @c AplAudioPlayerExtension.
 * The observer should control the state of the corresponding @c AudioPlayer:
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/audioplayer.html
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/playbackcontroller.html
 */
class AplAudioPlayerExtensionObserverInterface {
public:
    /**
     * Destructor
     */
    virtual ~AplAudioPlayerExtensionObserverInterface() = default;

    /**
     * Used to notify the observer when the extension has issued a Play event.
     * The observer should change @c AudioPlayer state to PLAYING.
     */
    virtual void onAudioPlayerPlay() = 0;

    /**
     * Used to notify the observer when the extension has issued a Pause event.
     * The observer should change @c AudioPlayer state to PAUSED.
     */
    virtual void onAudioPlayerPause() = 0;

    /**
     * Used to notify the observer when the extension has issued a Next event.
     * The observer should advance @c AudioPlayer to the NEXT item in the queue.
     */
    virtual void onAudioPlayerNext() = 0;

    /**
     * Used to notify the observer when the extension has issued a Previous event.
     * The observer should set @c AudioPlayer to the PREVIOUS item in the queue.
     */
    virtual void onAudioPlayerPrevious() = 0;

    /**
     * Used to notify the observer when the extension has issued a SkipToPosition event.
     * The observer should seek the @c AudioPlayer offset to the provided value.
     * @param offsetInMilliseconds Offset to skip to.
     */
    virtual void onAudioPlayerSeekToPosition(int offsetInMilliseconds) = 0;

    /**
     * Used to notify the observer when the extension has issued a Toggle event.
     * The observer should report the provided TOGGLE control state for the @c AudioPlayer.
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/playbackcontroller.html#togglecommandissued
     * @param name The name of the toggle.
     * @param checked The checked state of the toggle.
     */
    virtual void onAudioPlayerToggle(const std::string& name, bool checked) = 0;

    /**
     * Used to notify the observer when the extension has flushed lyrics viewed data.
     * The observer should report the provided LyricsViewed data for the @c AudioPlayer.Presentation
     * @param token Meta-information about the track that displayed lyrics.
     * @param durationInMilliseconds The time in milliseconds that lyrics were displayed.
     * @param lyricData Opaque serialized data object for reporting lyrics viewed.
     */
    virtual void onAudioPlayerLyricDataFlushed(const std::string& token,
                                               long durationInMilliseconds,
                                               const std::string& lyricData) = 0;

    /**
     * ADDITIONAL API - Supports Full AVS PlaybackController API usage.
     * Used to notify the observer when the extension has issued a SkipForward event.
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/playbackcontroller.html#buttoncommandissued
     * The observer should SKIPFORWARD in the @c AudioPlayer.
     */
    virtual void onAudioPlayerSkipForward() = 0;

    /**
     * ADDITIONAL API - Supports Full AVS PlaybackController API usage.
     * Used to notify the observer when the extension has issued a SkipBackward event.
     * The observer should SKIPBACKWARD in the @c AudioPlayer.
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/playbackcontroller.html#buttoncommandissued
     */
    virtual void onAudioPlayerSkipBackward() = 0;
};

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSIONOBSERVERINTERFACE_H
