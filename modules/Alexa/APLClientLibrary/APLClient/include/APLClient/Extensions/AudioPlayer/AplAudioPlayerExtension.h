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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSION_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSION_H

#include <iostream>

#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include "AplAudioPlayerExtensionObserverInterface.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

static const std::string URI = "aplext:audioplayer:10";

/**
 * An APL Extension designed for bi-directional communication between an @c AudioPlayer and APL document
 * to allow for control and command of audio stream and APL UI.
 */
class AplAudioPlayerExtension
        : public AplCoreExtensionInterface
        , public std::enable_shared_from_this<AplAudioPlayerExtension> {
public:
    /**
     * Constructor
     */
    AplAudioPlayerExtension(std::shared_ptr<AplAudioPlayerExtensionObserverInterface> observer);

    virtual ~AplAudioPlayerExtension() = default;

    /// @name AplCoreExtensionInterface Functions
    /// @{
    std::string getUri() override;

    apl::Object getEnvironment() override;

    std::list<apl::ExtensionCommandDefinition> getCommandDefinitions() override;

    std::list<apl::ExtensionEventHandler> getEventHandlers() override;

    std::unordered_map<std::string, apl::LiveObjectPtr> getLiveDataObjects() override;

    void applySettings(const apl::Object& settings) override;
    /// @}

    /// @name AplCoreExtensionEventCallbackInterface Functions
    /// @{
    void onExtensionEvent(
        const std::string& uri,
        const std::string& name,
        const apl::Object& source,
        const apl::Object& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback = nullptr) override;
    /// @}

    /**
     * Call to invoke the OnPlayerActivityUpdated ExtensionEventHandler and update the playbackState apl::LiveMap.
     * It is expected that this is called on every change in the AudioPlayer's PlayerActivity state.
     *
     * @param state The player activity state as defined in
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/audioplayer.html#context
     * @param offset The current offsetInMilliseconds for the active audioItem received from
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/audioplayer.html#play
     */
    void updatePlayerActivity(const std::string& state, int offset);

    /**
     * Call to update the audioItem offset property of the playbackState apl::LiveMap.
     * It is expected that this is called on every offset change (tick) from the AudioPlayer's audioItem to consistently
     * update playback progress.
     *
     * @param offset The current offsetInMilliseconds for the active audioItem received from
     * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/audioplayer.html#play
     */
    void updatePlaybackProgress(int offset);

    /**
     * Used to inform the extension of the active @c AudioPlayer.Presentation.APL presentationSession.
     * @param id The identifier of the active presentation session.
     * @param skillId The identifier of the active Skill / Speechlet who owns the session.
     */
    void setActivePresentationSession(const std::string& id, const std::string& skillId);

private:

    /**
     * Utility object for tracking lyrics viewed data.
     */
    struct LyricsViewedData {
        /**
         * Default Constructor.
         */
        LyricsViewedData() = default;

        /**
         * Constructor.
         *
         * @param token the identifier of the track displaying lyrics.
         */
        explicit LyricsViewedData(std::string token) : token{std::move(token)} {
            lyricData = std::make_shared<apl::ObjectArray>();
        };

        /// The identifier of the track displaying lyrics.
        std::string token;

        /// The total time in milliseconds that lyrics were viewed.
        long durationInMilliseconds{};

        /// The lyrics viewed data array.
        apl::ObjectArrayPtr lyricData;

        /**
         * Add Lyric lines to the data array.
         * @param lines The lines of lyrics to append.
         */
        void addLyricLinesData(const apl::ObjectArray& lines) const {
            /// List of valid lyrics property.
            const std::vector<std::string> validLyricPropertyName = {
                    "text",
                    "startTime",
                    "endTime"
            };
            for (const auto & line : lines) {
                apl::ObjectMapPtr m = std::make_shared<apl::ObjectMap>();
                for (const auto & it : line.getMap()) {
                    if (std::find(validLyricPropertyName.begin(), validLyricPropertyName.end(), it.first) != validLyricPropertyName.end()) {
                        m->emplace(it.first, it.second);
                    } else {
                        logMessage(apl::LogLevel::kWarn, "LyricsViewedData", __func__, "Ignoring invalid lyric property: " + it.first);
                    }
                }
                lyricData->emplace_back(apl::Object(m));
            }
        }

        /**
         * Resets the LyricsData object
         */
        void reset() {
            token = "";
            durationInMilliseconds = 0;
            lyricData->clear();
        }

        /**
         * Returns string payload of the lyricData object.
         * @return the lyricData object payload.
         */
        std::string getLyricDataPayload() const {
            rapidjson::Document lyricDataJsonDoc(rapidjson::kObjectType);
            auto& allocator = lyricDataJsonDoc.GetAllocator();
            auto lyricsObject = apl::Object(lyricData).serialize(allocator);
            for (rapidjson::Value::ValueIterator itr = lyricsObject.Begin(); itr != lyricsObject.End(); ++itr) { // Ok
                if (itr->HasMember("startTime")) {
                    (*itr)["startTime"].SetInt((int) (*itr)["startTime"].GetDouble());
                }
                if (itr->HasMember("endTime")) {
                    (*itr)["endTime"].SetInt((int) (*itr)["endTime"].GetDouble());
                }
            }

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            lyricsObject.Accept(writer);
            return buffer.GetString();
        }
    };

    /**
     * An internal function to retrieve the active @c LyricsViewedData object from the m_lyricsViewedData map
     * based on the m_activeSkillId;
     * @param initIfNull If true this will initialize a @c LyricsViewedData object for the m_activeSkillId if none exists.
     * @param token - The token for the track actively displaying lyrics.
     */
    std::shared_ptr<LyricsViewedData> getActiveLyricsViewedData(bool initIfNull = false, const std::string& token = "");

    /**
     * Flushes the provided @c LyricsViewedData and notifies the observer.
     * @param lyricsViewedData The @c LyricsViewedData to flush.
     */
    void flushLyricData(const std::shared_ptr<LyricsViewedData>& lyricsViewedData);

    /// The @c AplAudioPlayerExtensionObserverInterface observer
    std::shared_ptr<AplAudioPlayerExtensionObserverInterface> m_observer;

    /// The document settings defined 'name' for the playbackState data object
    std::string m_playbackStateName;

    /// The @c apl::LiveMap for AudioPlayer playbackState data.
    apl::LiveMapPtr m_playbackState;

    /// The id of the active skill in session.
    std::string m_activeSkillId;

    /// The map of @c LyricsViewedData objects per skill Id.
    std::unordered_map<std::string, std::shared_ptr<LyricsViewedData>> m_lyricsViewedData;
};

using AplAudioPlayerExtensionPtr = std::shared_ptr<AplAudioPlayerExtension>;

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSION_H
