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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSIONOBSERVERINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSIONOBSERVERINTERFACE_H

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/**
 * This class allows a @c AplAudioPlayerAlarmsExtensionObserverInterface observer to be notified of changes in the
 * @c AplAudioPlayerAlarmsExtension.
 * The observer should control the state of the corresponding @c AudioPlayer and/or @c Alerts API's
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/audioplayer.html
 * https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alerts.html
 */
class AplAudioPlayerAlarmsExtensionObserverInterface {
public:
    /**
     * Destructor
     */
    virtual ~AplAudioPlayerAlarmsExtensionObserverInterface() = default;

    /**
     * Used to notify the observer when the extension has issued a Dismiss event.
     * The observer should Dismiss/Stop the active @c AudioPlayer alarm.
     */
    virtual void onAudioPlayerAlarmDismiss() = 0;

    /**
     * Used to notify the observer when the extension has issued a Snooze event.
     * The observer should snooze the @c AudioPlayer alarm.
     */
    virtual void onAudioPlayerAlarmSnooze() = 0;
};

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSIONOBSERVERINTERFACE_H
