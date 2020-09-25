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

#include <climits>

#include "APLClient/AplCoreViewhostMessage.h"
#include "APLClient/AplCoreTextMeasurement.h"

namespace APLClient {

/// The keys used in APL text measurement.
static const char MEASURE_KEY[] = "measure";
static const char BASELINE_KEY[] = "baseline";

AplCoreTextMeasurement::AplCoreTextMeasurement(
        AplCoreConnectionManagerPtr aplCoreConnectionManager,
        AplConfigurationPtr config)
    : m_aplCoreConnectionManager{aplCoreConnectionManager},
      m_aplConfiguration{config} {

    m_textMeasureCounter = m_aplConfiguration->getMetricsRecorder()->createCounter(
            Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
   Telemetry::AplRenderingSegment::kTextMeasure);
}

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
apl::LayoutSize AplCoreTextMeasurement::measure(
        apl::Component* component,
        float width,
        apl::MeasureMode widthMode,
        float height,
        apl::MeasureMode heightMode) {
    auto aplOptions = m_aplConfiguration->getAplOptions();
    m_textMeasureCounter->increment();
    if (auto aplCoreConnectionManager = m_aplCoreConnectionManager.lock()) {
        /* Notify about the text measurement event */
        aplOptions->onRenderingEvent(aplCoreConnectionManager->getAPLToken(), AplRenderingEvent::TEXT_MEASURE);

        auto msg = AplCoreViewhostMessage(MEASURE_KEY);
        auto& alloc = msg.alloc();

        auto aplCoreMetrics = aplCoreConnectionManager->aplCoreMetrics();

        rapidjson::Value payload(component->serialize(alloc));
        payload.AddMember("width", aplCoreMetrics->toViewhost(std::isnan(width) ? INT_MAX : width), alloc);
        payload.AddMember("height", aplCoreMetrics->toViewhost(std::isnan(height) ? INT_MAX : height), alloc);
        payload.AddMember("widthMode", widthMode, alloc);
        payload.AddMember("heightMode", heightMode, alloc);
        msg.setPayload(std::move(payload));

        auto result = aplCoreConnectionManager->blockingSend(msg);

        if (result.IsObject()) {
            auto measuredWidth = aplCoreMetrics->toCore(result["payload"]["width"].GetFloat());
            auto measuredHeight = aplCoreMetrics->toCore(result["payload"]["height"].GetFloat());

            return {measuredWidth, measuredHeight};
        }

        aplOptions->logMessage(LogLevel::WARN, __func__, "Didn't get a valid reply.  Returning generic size.");
        return {aplCoreMetrics->toCore(100), aplCoreMetrics->toCore(100)};
    } else {
        aplOptions->logMessage(LogLevel::WARN, __func__, "ConnectionManager does not exist. Returning generic size.");
        return {0, 0};
    }
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
float AplCoreTextMeasurement::baseline(apl::Component* component, float width, float height) {
    if (auto aplCoreConnectionManager = m_aplCoreConnectionManager.lock()) {
        auto msg = AplCoreViewhostMessage(BASELINE_KEY);
        auto& alloc = msg.alloc();

        auto aplCoreMetrics = aplCoreConnectionManager->aplCoreMetrics();

        rapidjson::Value payload(rapidjson::kObjectType);
        payload.AddMember("id", rapidjson::Value(component->getUniqueId().c_str(), alloc).Move(), alloc);
        payload.AddMember("width", aplCoreMetrics->toViewhost(width), alloc);
        payload.AddMember("height", aplCoreMetrics->toViewhost(height), alloc);
        msg.setPayload(std::move(payload));

        auto result = aplCoreConnectionManager->blockingSend(msg);

        if (result.IsObject()) {
            auto it = result.FindMember("payload");
            if (it != result.MemberEnd()) return aplCoreMetrics->toCore(it->value.GetFloat());
        }
    }

    auto aplOptions = m_aplConfiguration->getAplOptions();
    aplOptions->logMessage(LogLevel::WARN, __func__, "Got invalid result from baseline calculation. Returning 0.");
    return 0;
}

}  // namespace APLClient
