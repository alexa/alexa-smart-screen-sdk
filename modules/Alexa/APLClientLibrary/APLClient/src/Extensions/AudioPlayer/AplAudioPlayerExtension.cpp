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

#include <string>
#include <utility>
#include "APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/// String to identify log entries originating from this file.
static const std::string TAG("AplAudioPlayerExtension");

static const std::string SETTING_PLAYBACK_STATE_NAME = "playbackStateName";
static const std::string COMMAND_PLAY_NAME = "Play";
static const std::string COMMAND_PAUSE_NAME = "Pause";
static const std::string COMMAND_PREVIOUS_NAME = "Previous";
static const std::string COMMAND_NEXT_NAME = "Next";
static const std::string COMMAND_SEEK_TO_POSITION_NAME = "SeekToPosition";
static const std::string COMMAND_TOGGLE_NAME = "Toggle";
static const std::string COMMAND_ADD_LYRICS_VIEWED = "AddLyricsViewed";
static const std::string COMMAND_ADD_LYRICS_DURATION_IN_MILLISECONDS = "AddLyricsDurationInMilliseconds";
static const std::string COMMAND_FLUSH_LYRIC_DATA = "FlushLyricData";
static const std::string COMMAND_SKIP_FORWARD_NAME = "SkipForward";
static const std::string COMMAND_SKIP_BACKWARD_NAME = "SkipBackward";
static const std::string EVENTHANDLER_ON_PLAYER_ACTIVITY_UPDATED_NAME = "OnPlayerActivityUpdated";
static const std::string PROPERTY_OFFSET = "offset";
static const std::string PROPERTY_TOGGLE_NAME = "name";
static const std::string PROPERTY_TOGGLE_CHECKED = "checked";
static const std::string PROPERTY_PLAYER_ACTIVITY = "playerActivity";
static const std::string PROPERTY_TOKEN = "token";
static const std::string PROPERTY_LINES = "lines";
static const std::string PROPERTY_DURATION_IN_MILLISECONDS = "durationInMilliseconds";

/// List of accepted toggle command names.
static const std::vector<std::string> TOGGLE_COMMAND_NAMES = {
    "thumbsUp",
    "thumbsDown",
    "shuffle",
    "repeat"
};

/// List of accepted player activity.
static const std::vector<std::string> PLAYER_ACTIVITY = {
    "PLAYING",
    "STOPPED",
    "PAUSED",
    "BUFFER_UNDERRUN"
};

AplAudioPlayerExtension::AplAudioPlayerExtension(std::shared_ptr<AplAudioPlayerExtensionObserverInterface> observer) :
        m_observer{std::move(observer)} {
    m_playbackStateName = "";
    m_activeSkillId = "";
    m_playbackState = apl::LiveMap::create();
    m_playbackState->set(PROPERTY_PLAYER_ACTIVITY, "STOPPED");
    m_playbackState->set(PROPERTY_OFFSET, 0);
}

std::string AplAudioPlayerExtension::getUri() {
    return URI;
}

apl::Object AplAudioPlayerExtension::getEnvironment() {
    // No environment for AudioPlayer Extension
    return apl::Object("");
}
std::list<apl::ExtensionCommandDefinition> AplAudioPlayerExtension::getCommandDefinitions() {
    std::list<apl::ExtensionCommandDefinition> extCmdDefs(
        {apl::ExtensionCommandDefinition(URI, COMMAND_PLAY_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_PAUSE_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_PREVIOUS_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_NEXT_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_SEEK_TO_POSITION_NAME)
             .allowFastMode(true)
             .property(PROPERTY_OFFSET, 0, true),
         apl::ExtensionCommandDefinition(URI, COMMAND_TOGGLE_NAME)
             .allowFastMode(true)
             .property(PROPERTY_TOGGLE_NAME, "", true)
             .property(PROPERTY_TOGGLE_CHECKED, false, true),
         apl::ExtensionCommandDefinition(URI, COMMAND_SKIP_FORWARD_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_SKIP_BACKWARD_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_ADD_LYRICS_VIEWED)
            .allowFastMode(true)
            .property(PROPERTY_TOKEN, "", true)
            .property(PROPERTY_LINES, apl::ObjectArray(), true),
         apl::ExtensionCommandDefinition(URI, COMMAND_ADD_LYRICS_DURATION_IN_MILLISECONDS)
            .allowFastMode(true)
            .property(PROPERTY_TOKEN, "", true)
            .property(PROPERTY_DURATION_IN_MILLISECONDS, 0, true),
         apl::ExtensionCommandDefinition(URI, COMMAND_FLUSH_LYRIC_DATA).allowFastMode(true)});
    return extCmdDefs;
}
std::list<apl::ExtensionEventHandler> AplAudioPlayerExtension::getEventHandlers() {
    std::list<apl::ExtensionEventHandler> extensionEventHandlers(
        {apl::ExtensionEventHandler(URI, EVENTHANDLER_ON_PLAYER_ACTIVITY_UPDATED_NAME)});
    return extensionEventHandlers;
}

std::unordered_map<std::string, apl::LiveObjectPtr> AplAudioPlayerExtension::getLiveDataObjects() {
    auto liveObjects = std::unordered_map<std::string, apl::LiveObjectPtr>();
    if (!m_playbackStateName.empty()) {
        liveObjects.emplace(m_playbackStateName, m_playbackState);
    }
    return liveObjects;
}

void AplAudioPlayerExtension::applySettings(const apl::Object& settings) {
    // Reset to defaults
    m_playbackStateName = "";
    /// Apply @c apl::Content defined settings
    logMessage(apl::LogLevel::kInfo, TAG, __func__, settings.toDebugString());
    if (settings.isMap()) {
        if (settings.has(SETTING_PLAYBACK_STATE_NAME)) {
            m_playbackStateName = settings.get(SETTING_PLAYBACK_STATE_NAME).getString();
        }
    }
}

void AplAudioPlayerExtension::onExtensionEvent(
    const std::string& uri,
    const std::string& name,
    const apl::Object& source,
    const apl::Object& params,
    unsigned int event,
    std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) {
    auto eventDebugString = getEventDebugString(uri, name, params);
    logMessage(LOGLEVEL_DEBUG, TAG, __func__, eventDebugString);

    bool succeeded = true;

    if (m_observer) {
        if (COMMAND_PLAY_NAME == name) {
            m_observer->onAudioPlayerPlay();
        } else if (COMMAND_PAUSE_NAME == name) {
            m_observer->onAudioPlayerPause();
        } else if (COMMAND_PREVIOUS_NAME == name) {
            m_observer->onAudioPlayerPrevious();
        } else if (COMMAND_NEXT_NAME == name) {
            m_observer->onAudioPlayerNext();
        } else if (COMMAND_SEEK_TO_POSITION_NAME == name) {
            if (confirmEventParams(TAG, {PROPERTY_OFFSET}, params)) {
                m_observer->onAudioPlayerSeekToPosition(params.get(PROPERTY_OFFSET).getInteger());
            } else {
                succeeded = false;
            }
        } else if (COMMAND_SKIP_FORWARD_NAME == name) {
            m_observer->onAudioPlayerSkipForward();
        } else if (COMMAND_SKIP_BACKWARD_NAME == name) {
            m_observer->onAudioPlayerSkipBackward();
        } else if (COMMAND_TOGGLE_NAME == name) {
            if (confirmEventParams(TAG, {PROPERTY_TOGGLE_NAME, PROPERTY_TOGGLE_CHECKED}, params)) {
                auto toggleName = params.get(PROPERTY_TOGGLE_NAME).getString();
                if (std::find(TOGGLE_COMMAND_NAMES.begin(), TOGGLE_COMMAND_NAMES.end(), toggleName) != TOGGLE_COMMAND_NAMES.end()) {
                    m_observer->onAudioPlayerToggle(toggleName, params.get(PROPERTY_TOGGLE_CHECKED).getBoolean());
                } else {
                    logMessage(apl::LogLevel::kError, TAG, __func__, "Invalid Toggle Command Name: " + eventDebugString);
                    succeeded = false;
                }
            } else {
                succeeded = false;
            }
        } else if (COMMAND_ADD_LYRICS_VIEWED == name) {
            if (confirmEventParams(TAG, {PROPERTY_TOKEN, PROPERTY_LINES}, params)) {
                auto tokenObject = params.get(PROPERTY_TOKEN);
                auto token = tokenObject.isString() ? tokenObject.getString() : "";
                if (auto lyricData = getActiveLyricsViewedData(true, token)) {
                    lyricData->addLyricLinesData(params.get(PROPERTY_LINES).getArray());
                }
            } else {
                succeeded = false;
            }
        } else if (COMMAND_ADD_LYRICS_DURATION_IN_MILLISECONDS == name) {
            if (confirmEventParams(TAG, {PROPERTY_TOKEN, PROPERTY_DURATION_IN_MILLISECONDS}, params)) {
                auto tokenObject = params.get(PROPERTY_TOKEN);
                auto token = tokenObject.isString() ? tokenObject.getString() : "";
                if (auto lyricData = getActiveLyricsViewedData(true, token)) {
                    lyricData->durationInMilliseconds += params.get(PROPERTY_DURATION_IN_MILLISECONDS).getDouble();
                }
            } else {
                succeeded = false;
            }
        } else if (COMMAND_FLUSH_LYRIC_DATA == name) {
            if (auto lyricData = getActiveLyricsViewedData()) {
                flushLyricData(lyricData);
            }
        } else {
            logMessage(apl::LogLevel::kError, TAG, __func__, "Invalid Command: " + eventDebugString);
            succeeded = false;
        }
    } else {
        logMessage(apl::LogLevel::kError, TAG, __func__, "No Event Observer: " + eventDebugString);
        succeeded = false;
    }

    if (resultCallback) {
        resultCallback->onExtensionEventResult(event, succeeded);
    }
}

std::shared_ptr<AplAudioPlayerExtension::LyricsViewedData> AplAudioPlayerExtension::getActiveLyricsViewedData(bool initIfNull, const std::string& token) {
    if (!m_activeSkillId.empty()) {
        auto lvdi = m_lyricsViewedData.find(m_activeSkillId);
        if (lvdi != m_lyricsViewedData.end()) {
            auto lyricsViewedData = lvdi->second;
            // If token has changed for the active skill's lyric data, flush the data and set the new token.
            if (!token.empty() && lyricsViewedData->token != token) {
                flushLyricData(lyricsViewedData);
                lyricsViewedData->token = token;
            }
            return lyricsViewedData;
        }
    }

    if (initIfNull) {
        m_lyricsViewedData[m_activeSkillId] = std::make_shared<LyricsViewedData>(token);
        return m_lyricsViewedData[m_activeSkillId];
    }

    return nullptr;
}

void AplAudioPlayerExtension::flushLyricData(const std::shared_ptr<LyricsViewedData>& lyricsViewedData) {
    if (!lyricsViewedData->lyricData->empty()) {
        m_observer->onAudioPlayerLyricDataFlushed(lyricsViewedData->token,
                                                  lyricsViewedData->durationInMilliseconds,
                                                  lyricsViewedData->getLyricDataPayload());
    }
    lyricsViewedData->reset();
}

void AplAudioPlayerExtension::updatePlayerActivity(const std::string& state, int offset) {
    if (std::find(PLAYER_ACTIVITY.begin(), PLAYER_ACTIVITY.end(), state) == PLAYER_ACTIVITY.end()) {
        logMessage(apl::LogLevel::kError, TAG, __func__, "Invalid Player Activity: " + state);
        return;
    }

    m_playbackState->set(PROPERTY_PLAYER_ACTIVITY, state);
    m_playbackState->set(PROPERTY_OFFSET, offset);

    if (!m_eventHandler) {
        logMessage(apl::LogLevel::kWarn, TAG, __func__, "No Event Handler");
        return;
    }

    auto playerActivity = apl::ObjectMap({{PROPERTY_PLAYER_ACTIVITY, state}});

    m_eventHandler->invokeExtensionEventHandler(
        URI, EVENTHANDLER_ON_PLAYER_ACTIVITY_UPDATED_NAME, playerActivity, false);
}

void AplAudioPlayerExtension::updatePlaybackProgress(int offset) {
    m_playbackState->set(PROPERTY_OFFSET, offset);
}

void AplAudioPlayerExtension::setActivePresentationSession(const std::string &id, const std::string &skillId) {
    m_activeSkillId = skillId;
    /// If there's available lyricsViewedData for the newly active skillId, report it immediately
    if (auto lyricsViewedData = getActiveLyricsViewedData()) {
        flushLyricData(lyricsViewedData);
    }
}

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient
