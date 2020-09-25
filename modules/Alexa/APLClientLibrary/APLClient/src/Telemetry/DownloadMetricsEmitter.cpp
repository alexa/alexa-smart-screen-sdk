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

#include "APLClient/Telemetry/DownloadMetricsEmitter.h"

#include <string>

namespace APLClient {
namespace Telemetry {

DownloadMetricsEmitter::DownloadMetricsEmitter(AplMetricsRecorderInterfacePtr metricsRecorder)
       : m_metricsRecorder{metricsRecorder} {
    m_cacheCounter = m_metricsRecorder->createCounter(
            AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "SmartScreenSDK.ImportDocumentCacheHit");
    m_sizeCounter = m_metricsRecorder->createCounter(
            AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "SmartScreenSDK.ImportDocumentSize", false);
    auto importCounter = m_metricsRecorder->createCounter(
            AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "SmartScreenSDK.ImportDocument");
    importCounter->increment();
}

void DownloadMetricsEmitter::onDownloadStarted() {
    m_downloadTimer = m_metricsRecorder->createTimer(
            APLClient::Telemetry::AplMetricsRecorderInterface::LATEST_DOCUMENT,
            "SmartScreenSDK.ImportDocumentTime");
    m_downloadTimer->start();
}

void DownloadMetricsEmitter::onDownloadComplete()  {
    m_downloadTimer->stop();
}

void DownloadMetricsEmitter::onDownloadFailed()  {
    m_downloadTimer->fail();
}

void DownloadMetricsEmitter::onCacheHit() {
    m_cacheCounter->increment();
}

void DownloadMetricsEmitter::onBytesRead(std::uint64_t numberOfBytes)  {
    m_sizeCounter->incrementBy(numberOfBytes);
}

} // namespace Telemetry
} // namespace APLClient
