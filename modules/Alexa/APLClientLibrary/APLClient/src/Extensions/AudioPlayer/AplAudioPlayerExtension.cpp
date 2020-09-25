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
#include "APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/// String to identify log entries originating from this file.
static const std::string TAG("AplAudioPlayerExtension");

static const std::string URI = "aplext:audioplayer:10";
static const std::string SETTING_PLAYBACK_STATE_NAME = "playbackStateName";
static const std::string COMMAND_PLAY_NAME = "Play";
static const std::string COMMAND_PAUSE_NAME = "Pause";
static const std::string COMMAND_PREVIOUS_NAME = "Previous";
static const std::string COMMAND_NEXT_NAME = "Next";
static const std::string COMMAND_SEEK_TO_POSITION_NAME = "SeekToPosition";
static const std::string COMMAND_TOGGLE_NAME = "Toggle";
static const std::string COMMAND_SKIP_FORWARD_NAME = "SkipForward";
static const std::string COMMAND_SKIP_BACKWARD_NAME = "SkipBackward";
static const std::string EVENTHANDLER_ON_PLAYER_ACTIVITY_UPDATED_NAME = "OnPlayerActivityUpdated";
static const std::string PROPERTY_OFFSET = "offset";
static const std::string PROPERTY_TOGGLE_NAME = "name";
static const std::string PROPERTY_TOGGLE_CHECKED = "checked";
static const std::string PROPERTY_PLAYER_ACTIVITY = "playerActivity";

AplAudioPlayerExtension::AplAudioPlayerExtension(std::shared_ptr<AplAudioPlayerExtensionObserverInterface> observer) :
        m_observer{observer} {
    m_playbackStateName = "";
    m_playbackState = apl::LiveMap::create();
    m_playbackState->set(PROPERTY_PLAYER_ACTIVITY, "IDLE");
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
         apl::ExtensionCommandDefinition(URI, COMMAND_SKIP_FORWARD_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_SKIP_BACKWARD_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_TOGGLE_NAME)
             .allowFastMode(true)
             .property(PROPERTY_TOGGLE_NAME, "", true)
             .property(PROPERTY_TOGGLE_CHECKED, false, true)});
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
    logMessage(apl::LogLevel::INFO, TAG, __func__, settings.toDebugString());
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
                m_observer->onAudioPlayerToggle(
                    params.get(PROPERTY_TOGGLE_NAME).getString(), params.get(PROPERTY_TOGGLE_CHECKED).getBoolean());
            } else {
                succeeded = false;
            }
        } else {
            logMessage(apl::LogLevel::ERROR, TAG, __func__, "Invalid Command: " + eventDebugString);
            succeeded = false;
        }
    } else {
        logMessage(apl::LogLevel::ERROR, TAG, __func__, "No Event Observer: " + eventDebugString);
        succeeded = false;
    }

    if (resultCallback) {
        resultCallback->onExtensionEventResult(event, succeeded);
    }
}

void AplAudioPlayerExtension::updatePlayerActivity(const std::string& state, int offset) {
    m_playbackState->set(PROPERTY_PLAYER_ACTIVITY, state);
    m_playbackState->set(PROPERTY_OFFSET, offset);

    if (!m_eventHandler) {
        logMessage(apl::LogLevel::WARN, TAG, __func__, "No Event Handler");
        return;
    }

    auto playerActivity = apl::ObjectMap({{PROPERTY_PLAYER_ACTIVITY, state}});

    m_eventHandler->invokeExtensionEventHandler(
        URI, EVENTHANDLER_ON_PLAYER_ACTIVITY_UPDATED_NAME, playerActivity, false);
}

void AplAudioPlayerExtension::updatePlaybackProgress(int offset) {
    m_playbackState->set(PROPERTY_OFFSET, offset);
}

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient
