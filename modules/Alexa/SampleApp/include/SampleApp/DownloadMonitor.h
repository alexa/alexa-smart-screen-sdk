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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DOWNLOADMONITOR_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DOWNLOADMONITOR_H

#include <APLClient/Telemetry/DownloadMetricsEmitter.h>

#include "SampleApp/CachingDownloadManager.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * Emits APL telemetry in response to download manager events.
 */
class DownloadMonitor : public alexaSmartScreenSDK::sampleApp::CachingDownloadManager::Observer {
public:
    explicit DownloadMonitor(APLClient::Telemetry::DownloadMetricsEmitterPtr metricsEmitter);

    ~DownloadMonitor() override = default;

    void onDownloadStarted() override;

    void onDownloadComplete() override;

    void onDownloadFailed() override;

    void onCacheHit() override;

    void onBytesRead(uint64_t numberOfBytes) override;

private:
    APLClient::Telemetry::DownloadMetricsEmitterPtr m_metricsEmitter;
};

} // namespace sampleApp
} // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DOWNLOADMONITOR_H
