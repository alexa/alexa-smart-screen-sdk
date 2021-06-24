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
#include "APLClient/Extensions/AudioPlayer/AplAudioPlayerAlarmsExtension.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/// String to identify log entries originating from this file.
static const std::string TAG("AplAudioPlayerAlarmsExtension");

static const std::string URI = "aplext:musicalarm:10";
static const std::string COMMAND_DISMISS_NAME = "DismissAlarm";
static const std::string COMMAND_SNOOZE_NAME = "SnoozeAlarm";

AplAudioPlayerAlarmsExtension::AplAudioPlayerAlarmsExtension(std::shared_ptr<AplAudioPlayerAlarmsExtensionObserverInterface> observer) :
        m_observer{std::move(observer)} {
}

std::string AplAudioPlayerAlarmsExtension::getUri() {
    return URI;
}

apl::Object AplAudioPlayerAlarmsExtension::getEnvironment() {
    // No environment for AudioPlayer Extension
    return apl::Object("");
}
std::list<apl::ExtensionCommandDefinition> AplAudioPlayerAlarmsExtension::getCommandDefinitions() {
    std::list<apl::ExtensionCommandDefinition> extCmdDefs(
        {apl::ExtensionCommandDefinition(URI, COMMAND_DISMISS_NAME).allowFastMode(true),
         apl::ExtensionCommandDefinition(URI, COMMAND_SNOOZE_NAME).allowFastMode(true)});
    return extCmdDefs;
}

std::list<apl::ExtensionEventHandler> AplAudioPlayerAlarmsExtension::getEventHandlers() {
    std::list<apl::ExtensionEventHandler> extensionEventHandlers({});
    return extensionEventHandlers;
}

std::unordered_map<std::string, apl::LiveObjectPtr> AplAudioPlayerAlarmsExtension::getLiveDataObjects() {
    auto liveObjects = std::unordered_map<std::string, apl::LiveObjectPtr>();
    return liveObjects;
}

void AplAudioPlayerAlarmsExtension::applySettings(const apl::Object& settings) {
    /// Apply @c apl::Content defined settings
    logMessage(apl::LogLevel::kInfo, TAG, __func__, settings.toDebugString());
}

void AplAudioPlayerAlarmsExtension::onExtensionEvent(
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
        if (COMMAND_DISMISS_NAME == name) {
            m_observer->onAudioPlayerAlarmDismiss();
        } else if (COMMAND_SNOOZE_NAME == name) {
            m_observer->onAudioPlayerAlarmSnooze();
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

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient
