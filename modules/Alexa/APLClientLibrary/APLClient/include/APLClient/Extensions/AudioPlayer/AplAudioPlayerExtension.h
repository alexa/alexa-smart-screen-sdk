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

#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include "AplAudioPlayerExtensionObserverInterface.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/**
 * An APL Extension designed for bi-directional communication between an @c AudioPlayer and APL document
 * to allow for control and command of audio stream and APL UI.
 *
 * TODO : Add link to public spec when available -
 * https://aplspec.aka.corp.amazon.com/apl-extensions-release/html/extensions/audioplayer/audioplayer_extension_10.html
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

private:
    /// The @c AplAudioPlayerExtensionObserverInterface observer
    std::shared_ptr<AplAudioPlayerExtensionObserverInterface> m_observer;

    /// The document settings defined 'name' for the playbackState data object
    std::string m_playbackStateName;

    /// The @c apl::LiveMap for AudioPlayer playbackState data.
    apl::LiveMapPtr m_playbackState;
};

using AplAudioPlayerExtensionPtr = std::shared_ptr<AplAudioPlayerExtension>;

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYEREXTENSION_H
