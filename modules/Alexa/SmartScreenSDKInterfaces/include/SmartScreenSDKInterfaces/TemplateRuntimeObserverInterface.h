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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIMEOBSERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIMEOBSERVERINTERFACE_H_

#include <chrono>
#include <string>

#include <AVSCommon/AVS/FocusState.h>
#include <AVSCommon/SDKInterfaces/MediaPropertiesInterface.h>

#include "AudioPlayerInfo.h"

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * This @c TemplateRuntimeObserverInterface class is used to notify observers when a @c RenderTemplate or
 * @c RenderPlayerInfo directive is received.  These two directives contains metadata for rendering display
 * cards for devices with GUI support.
 */
class TemplateRuntimeObserverInterface {
public:
    /**
     * Destructor
     */
    virtual ~TemplateRuntimeObserverInterface() = default;

    /**
     * Used to notify the observer when a RenderTemplate directive is received. Once called, the client should
     * render the Template display card based on the metadata provided in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param jsonPayload The payload of the RenderTemplate directive in structured JSON format.
     * @param focusState The @c FocusState of the channel used by TemplateRuntime interface.
     */
    virtual void renderTemplateCard(
        const std::string& jsonPayload,
        alexaClientSDK::avsCommon::avs::FocusState focusState) = 0;

    /**
     * Used to notify the observer when the client should clear the Template display card.  Once the card is cleared,
     * the client should call templateCardCleared().
     *
     * @param token Token associated to the template card
     */
    virtual void clearTemplateCard(const std::string& aplToken) = 0;

    /**
     * Used to notify the observer when a RenderPlayerInfo directive is received. Once called, the client should
     * render the PlayerInfo display card based on the metadata provided in the payload in structured JSON
     * format.
     *
     * @param jsonPayload The payload of the RenderPlayerInfo directive in structured JSON format.
     * @param audioPlayerInfo Information on the @c AudioPlayer.
     * @param focusState The @c FocusState of the channel used by TemplateRuntime interface.
     * @param mediaProperties The @c MediaPropertiesInterface for the current @c AudioPlayer
     */
    virtual void renderPlayerInfoCard(
        const std::string& jsonPayload,
        smartScreenSDKInterfaces::AudioPlayerInfo audioPlayerInfo,
        alexaClientSDK::avsCommon::avs::FocusState focusState,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> mediaProperties) = 0;

    /**
     * Used to notify the observer when the client should clear the PlayerInfo display card.  Once the card is cleared,
     * the client should call templateCardCleared().
     *
     * @param token token associated to the player info card
     */
    virtual void clearPlayerInfoCard(const std::string& token) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_TEMPLATERUNTIME_INCLUDE_TEMPLATERUNTIME_TEMPLATERUNTIMEOBSERVERINTERFACE_H_
