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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONPRESENTER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONPRESENTER_H_

#include <SmartScreenSDKInterfaces/RenderCaptionsInterface.h>
#include <Captions/CaptionPresenterInterface.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

class SmartScreenCaptionPresenter : public alexaClientSDK::captions::CaptionPresenterInterface {
public:
    explicit SmartScreenCaptionPresenter(
        std::shared_ptr<smartScreenSDKInterfaces::RenderCaptionsInterface> renderCaptionsInterface);

    /// @name CaptionPresenterInterface methods
    /// @{
    void onCaptionActivity(
        const alexaClientSDK::captions::CaptionFrame& captionFrame,
        alexaClientSDK::avsCommon::avs::FocusState focusState) override;
    std::pair<bool, int> getWrapIndex(const alexaClientSDK::captions::CaptionLine& captionLine) override;
    ///@}

private:
    /// Pointer to the GUI Client interface
    std::shared_ptr<smartScreenSDKInterfaces::RenderCaptionsInterface> m_renderCaptionsInterface;

    rapidjson::Document convertCaptionFrameToJson(const alexaClientSDK::captions::CaptionFrame& captionFrame);
    rapidjson::Value convertCaptionLineToJson(
        const alexaClientSDK::captions::CaptionLine& captionLine,
        rapidjson::Document::AllocatorType& allocator);
    rapidjson::Value convertTextStyleToJson(
        alexaClientSDK::captions::TextStyle textStyle,
        rapidjson::Document::AllocatorType& allocator);
    rapidjson::Value convertStyleToJson(
        alexaClientSDK::captions::Style style,
        rapidjson::Document::AllocatorType& allocator);

    rapidjson::Value convertCaptionLinesToJson(
        const std::vector<alexaClientSDK::captions::CaptionLine>& captionLines,
        rapidjson::Document::AllocatorType& allocator);
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONPRESENTER_H_
