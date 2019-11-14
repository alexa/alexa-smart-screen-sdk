/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Messages/AplCoreViewhostMessage.h"
#include "SampleApp/AplCoreTextMeasurement.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

static const std::string TAG{"AplCoreTextMeasurement"};

/// The keys used in APL text measurement.
static const char MEASURE_KEY[] = "measure";
static const char BASELINE_KEY[] = "baseline";

#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/**
 * Request a text measurement.
 *
 *     { "type": "measure",
 *       "payload": {
 *           "id": UNIQUE_ID,
 *           "width": FLOAT,
 *           "height": FLOAT,
 *           "widthMode": INT,
 *           "heightMode": INT
 *           ....
 *      }}
 *
 * The response:
 *
 *     { "type": "measure",
 *       "payload": {
 *           "width": FLOAT,
 *           "height": FLOAT
 *     }}
 *
 * @param component
 * @param width
 * @param widthMode
 * @param height
 * @param heightMode
 * @return
 */
YGSize AplCoreTextMeasurement::measure(
    apl::TextComponent* component,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode) {
    auto msg = messages::AplCoreViewhostMessage(MEASURE_KEY);
    auto& alloc = msg.alloc();

    auto aplCoreMetrics = m_AplCoreConnectionManager->aplCoreMetrics();

    rapidjson::Value payload(component->serialize(alloc));
    payload.AddMember("width", aplCoreMetrics->toViewhost(width), alloc);
    payload.AddMember("height", aplCoreMetrics->toViewhost(height), alloc);
    payload.AddMember("widthMode", widthMode, alloc);
    payload.AddMember("heightMode", heightMode, alloc);
    msg.setPayload(std::move(payload));

    auto result = m_AplCoreConnectionManager->blockingSend(msg);

    if (result.IsObject()) {
        auto measuredWidth = aplCoreMetrics->toCore(result["payload"]["width"].GetFloat());
        auto measuredHeight = aplCoreMetrics->toCore(result["payload"]["height"].GetFloat());

        return {.width = measuredWidth, .height = measuredHeight};
    }

    ACSDK_WARN(LX(__func__).m("Didn't get a valid reply.  Returning generic size."));
    return {.width = aplCoreMetrics->toCore(100), .height = aplCoreMetrics->toCore(100)};
}

/**
 * Send a message to the view host asking for a baseline calculation:
 *
 *     { "type": "baseline",
 *       "payload": {
 *           "id": UNIQUE_ID,
 *           "width": FLOAT,
 *           "height": FLOAT }}
 *
 * The result should look like:
 *
 *     { "type": "baseline",
 *       "payload": FLOAT }
 *
 * @param component
 * @param width
 * @param height
 * @return
 */
float AplCoreTextMeasurement::baseline(apl::TextComponent* component, float width, float height) {
    auto msg = messages::AplCoreViewhostMessage(BASELINE_KEY);
    auto& alloc = msg.alloc();

    auto aplCoreMetrics = m_AplCoreConnectionManager->aplCoreMetrics();

    rapidjson::Value payload(rapidjson::kObjectType);
    payload.AddMember("id", rapidjson::Value(component->getUniqueId().c_str(), alloc).Move(), alloc);
    payload.AddMember("width", aplCoreMetrics->toViewhost(width), alloc);
    payload.AddMember("height", aplCoreMetrics->toViewhost(height), alloc);
    msg.setPayload(std::move(payload));

    auto result = m_AplCoreConnectionManager->blockingSend(msg);

    if (result.IsObject()) {
        auto it = result.FindMember("payload");
        if (it != result.MemberEnd()) return aplCoreMetrics->toCore(it->value.GetFloat());
    }

    ACSDK_WARN(LX(__func__).m("Got invalid result from baseline calculation. Returning 0."));
    return 0;
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK