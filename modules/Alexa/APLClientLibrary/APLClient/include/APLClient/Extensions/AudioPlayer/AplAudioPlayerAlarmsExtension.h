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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSION_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSION_H

#include "APLClient/Extensions/AplCoreExtensionInterface.h"
#include "AplAudioPlayerAlarmsExtensionObserverInterface.h"

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {

/**
 * An APL Extension designed for control of an @c AudioPlayer playing an Alarm from an APL document.
 */
class AplAudioPlayerAlarmsExtension
        : public AplCoreExtensionInterface
        , public std::enable_shared_from_this<AplAudioPlayerAlarmsExtension> {
public:
    /**
     * Constructor
     */
    AplAudioPlayerAlarmsExtension(std::shared_ptr<AplAudioPlayerAlarmsExtensionObserverInterface> observer);

    virtual ~AplAudioPlayerAlarmsExtension() = default;

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

private:

    /// The @c AplAudioPlayerAlarmsExtensionObserverInterface observer
    std::shared_ptr<AplAudioPlayerAlarmsExtensionObserverInterface> m_observer;
};

using AplAudioPlayerAlarmsExtensionPtr = std::shared_ptr<AplAudioPlayerAlarmsExtension>;

}  // namespace AudioPlayer
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_AUDIOPLAYER_APLAUDIOPLAYERALARMSEXTENSION_H
