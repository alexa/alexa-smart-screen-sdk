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

#ifndef APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_SINK_INTERFACE
#define APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_SINK_INTERFACE

#include <stdint.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>

namespace APLClient {
namespace Telemetry {

class AplMetricsSinkInterface {
public:
    AplMetricsSinkInterface() = default;
    virtual ~AplMetricsSinkInterface() = default;

    virtual void reportTimer(const std::map<std::string, std::string> &metadata,
                             const std::string& name,
                             const std::chrono::nanoseconds& value) = 0;
    virtual void reportCounter(const std::map<std::string, std::string> &metadata,
                               const std::string& name,
                               uint64_t value) = 0;
};

using AplMetricsSinkInterfacePtr = std::shared_ptr<AplMetricsSinkInterface>;

} // namespace Telemetry
} // namespace APLClient

#endif // APL_CLIENT_LIBRARY_TELEMETRY_APL_METRICS_SINK_INTERFACE
