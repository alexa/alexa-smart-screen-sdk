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

#ifndef APL_CLIENT_LIBRARY_APL_RENDERING_EVENT_OBSERVER_H_
#define APL_CLIENT_LIBRARY_APL_RENDERING_EVENT_OBSERVER_H_

#include <chrono>
#include <string>
#include <memory>
#include <vector>

#include "AplRenderingEvent.h"
#include "Telemetry/AplMetricsSinkInterface.h"

namespace APLClient {

class AplRenderingEventObserver {
public:
    AplRenderingEventObserver() = default;
    virtual ~AplRenderingEventObserver() = default;

    /**
     * Called when a RenderDocument directive is received
     *
     * @param receiveTime The earliest timestamp that the application became able to access the
     *                    directive.
     */
     virtual void onRenderDirectiveReceived(
            const std::chrono::steady_clock::time_point &receiveTime) = 0;

    /**
     * Called when a rendering event happened.
     *
     * @param event The rendering event
     */
    virtual void onRenderingEvent(AplRenderingEvent event) = 0;

    /**
     * Called when display metrics are reported by the viewhost.
     *
     * @param jsonPayload the reported metrics payload
     */
    virtual void onMetricsReported(const std::string& jsonPayload) = 0;

    /**
     * Called when the telemetry sink is updated.
     *
     * @param sink the new sink to use for telemetry.
     */
    virtual void onTelemetrySinkUpdated(APLClient::Telemetry::AplMetricsSinkInterfacePtr sink) = 0;
};

using AplRenderingEventObserverPtr = std::shared_ptr<AplRenderingEventObserver>;

}  // namespace APLClient

#endif  // APL_CLIENT_LIBRARY_APL_RENDERING_EVENT_OBSERVER_H_
