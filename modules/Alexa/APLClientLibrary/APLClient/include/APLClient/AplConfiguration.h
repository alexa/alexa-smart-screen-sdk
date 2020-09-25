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

#ifndef APL_CLIENT_LIBRARY_APL_CONFIGURATION_H_
#define APL_CLIENT_LIBRARY_APL_CONFIGURATION_H_

#include <memory>

#include "AplOptionsInterface.h"
#include "Telemetry/AplMetricsRecorderInterface.h"

namespace APLClient {

/**
 * Contains the set of configurable options for APL.
 */
class AplConfiguration final {
public:
    AplConfiguration(
            AplOptionsInterfacePtr options,
            Telemetry::AplMetricsRecorderInterfacePtr metricsRecorder = nullptr);

    ~AplConfiguration() = default;

    /**
     * Returns the current @c AplOptionsInterface instance.
     *
     * @return the current @c AplOptionsInterface instance
     */
    AplOptionsInterfacePtr getAplOptions() const;


    /**
     * Returns the currently configured metrics recorder. This is never null.
     *
     * @return the current metrics recorder to use for telemetry
     */
    Telemetry::AplMetricsRecorderInterfacePtr getMetricsRecorder() const;

    /**
     * Updates the currently configured metrics recoder. This has no effect
     * if @c nullptr is passed as a param.
     *
     * @param metricsRecorder the new metrics recorder to use
     */
    void setMetricsRecorder(Telemetry::AplMetricsRecorderInterfacePtr metricsRecorder);

private:
    AplOptionsInterfacePtr m_aplOptions;
    Telemetry::AplMetricsRecorderInterfacePtr m_metricsRecorder;
};

/// Convenience typedef
using AplConfigurationPtr = std::shared_ptr<AplConfiguration>;

}  // namespace APLClient
#endif  // APL_CLIENT_LIBRARY_APL_CONFIGURATION_H_
