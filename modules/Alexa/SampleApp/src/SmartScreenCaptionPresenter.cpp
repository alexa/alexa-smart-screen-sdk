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

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "SampleApp/SmartScreenCaptionPresenter.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

static const std::string TAG{"SmartScreenCaptionPresenter"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

SmartScreenCaptionPresenter::SmartScreenCaptionPresenter(
    std::shared_ptr<smartScreenSDKInterfaces::RenderCaptionsInterface> renderCaptionsInterface) {
    m_renderCaptionsInterface = renderCaptionsInterface;
}

void SmartScreenCaptionPresenter::onCaptionActivity(
    const alexaClientSDK::captions::CaptionFrame& captionFrame,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    if (alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND == focusState) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        convertCaptionFrameToJson(captionFrame).Accept(writer);
        m_renderCaptionsInterface->renderCaptions(buffer.GetString());
    }
}

std::pair<bool, int> SmartScreenCaptionPresenter::getWrapIndex(
    const alexaClientSDK::captions::CaptionLine& captionLine) {
    // Line wrap will be implemented on the GUI side
    return std::make_pair(false, 0);
}

rapidjson::Document SmartScreenCaptionPresenter::convertCaptionFrameToJson(
    const alexaClientSDK::captions::CaptionFrame& captionFrame) {
    rapidjson::Document captionFrameJson(rapidjson::kObjectType);

    captionFrameJson.AddMember(
        "duration",
        rapidjson::Value(std::chrono::duration_cast<std::chrono::milliseconds>(captionFrame.getDuration()).count())
            .Move(),
        captionFrameJson.GetAllocator());

    captionFrameJson.AddMember(
        "delay",
        rapidjson::Value(std::chrono::duration_cast<std::chrono::milliseconds>(captionFrame.getDelay()).count()).Move(),
        captionFrameJson.GetAllocator());

    captionFrameJson.AddMember(
        "captionLines",
        rapidjson::Value(
            convertCaptionLinesToJson(captionFrame.getCaptionLines(), captionFrameJson.GetAllocator()),
            captionFrameJson.GetAllocator()),
        captionFrameJson.GetAllocator());

    return captionFrameJson;
}

rapidjson::Value SmartScreenCaptionPresenter::convertCaptionLinesToJson(
    const std::vector<alexaClientSDK::captions::CaptionLine>& captionLines,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value captionLinesJson(rapidjson::kArrayType);
    for (const auto& captionLine : captionLines) {
        captionLinesJson.PushBack(convertCaptionLineToJson(captionLine, allocator), allocator);
    }

    return captionLinesJson;
}

rapidjson::Value SmartScreenCaptionPresenter::convertCaptionLineToJson(
    const alexaClientSDK::captions::CaptionLine& captionLine,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value captionLineJson(rapidjson::kObjectType);
    rapidjson::Value captionStyleArrayJson(rapidjson::kArrayType);

    for (auto textStyle : captionLine.styles) {
        captionStyleArrayJson.PushBack(convertTextStyleToJson(textStyle, allocator), allocator);
    }

    captionLineJson.AddMember("text", rapidjson::Value(captionLine.text, allocator), allocator);

    captionLineJson.AddMember("styles", rapidjson::Value(captionStyleArrayJson, allocator), allocator);

    return captionLineJson;
}

rapidjson::Value SmartScreenCaptionPresenter::convertTextStyleToJson(
    alexaClientSDK::captions::TextStyle textStyle,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value textStyleJson(rapidjson::kObjectType);

    textStyleJson.AddMember(
        "activeStyle",
        rapidjson::Value(convertStyleToJson(textStyle.activeStyle, allocator), allocator).Move(),
        allocator);

    textStyleJson.AddMember(
        "charIndex", rapidjson::Value(std::to_string(textStyle.charIndex), allocator).Move(), allocator);

    return textStyleJson;
}

rapidjson::Value SmartScreenCaptionPresenter::convertStyleToJson(
    alexaClientSDK::captions::Style style,
    rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value styleJson(rapidjson::kObjectType);

    styleJson.AddMember("bold", rapidjson::Value(std::to_string(style.m_bold), allocator).Move(), allocator);

    styleJson.AddMember("italic", rapidjson::Value(std::to_string(style.m_italic), allocator).Move(), allocator);

    styleJson.AddMember("underline", rapidjson::Value(std::to_string(style.m_underline), allocator).Move(), allocator);

    return styleJson;
}
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK