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

#ifndef APL_CLIENT_LIBRARY_TELEMETRY_DOWNLOAD_METRICS_EMITTER_H_
#define APL_CLIENT_LIBRARY_TELEMETRY_DOWNLOAD_METRICS_EMITTER_H_

#include <cstdint>
#include <memory>

#include "AplMetricsRecorderInterface.h"

namespace APLClient {
namespace Telemetry {

/**
 * Emits APL telemetry in response to download manager events.
 */
class DownloadMetricsEmitter final {
public:
    explicit DownloadMetricsEmitter(AplMetricsRecorderInterfacePtr metricsRecorder);

    ~DownloadMetricsEmitter() = default;

    /**
     * Called at the start of a download, when a resource is not found in the cache.
     */
    void onDownloadStarted();

    /**
     * Called when a resource was not found in the cache and has successfully been downloaded.
     */
    void onDownloadComplete();

    /**
     * Called when a resource was not found in the cache and the attempt to download it has failed.
     */
    void onDownloadFailed();

    /**
     * Called when a resource was found in the cache and downloading is not attempted.
     */
    void onCacheHit();

    /**
     * Called during the download of a resource. Observers should expect multiple calls
     * to this method for a single download.
     *
     * @param numberOfBytes The number of bytes that have been downloaded.
     */
    void onBytesRead(std::uint64_t numberOfBytes);

private:
    AplMetricsRecorderInterfacePtr m_metricsRecorder;
    std::unique_ptr<AplTimerHandle> m_downloadTimer;
    std::unique_ptr<AplCounterHandle> m_cacheCounter;
    std::unique_ptr<AplCounterHandle> m_sizeCounter;
};

using DownloadMetricsEmitterPtr = std::shared_ptr<DownloadMetricsEmitter>;

} // namespace Telemetry
} // namespace APLClient

#endif  // APL_CLIENT_LIBRARY_TELEMETRY_DOWNLOAD_METRICS_EMITTER_H_
