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

#include "APLClient/AplConfiguration.h"
#include "APLClient/Telemetry/NullAplMetricsRecorder.h"

namespace APLClient {

AplConfiguration::AplConfiguration(AplOptionsInterfacePtr options,
                                 Telemetry::AplMetricsRecorderInterfacePtr metricsRecorder)
        : m_aplOptions{options},
          m_metricsRecorder{metricsRecorder} {
    if (!m_metricsRecorder) {
        m_metricsRecorder = std::make_shared<Telemetry::NullAplMetricsRecorder>();
    }
}

AplOptionsInterfacePtr AplConfiguration::getAplOptions() const {
    return m_aplOptions;
}

Telemetry::AplMetricsRecorderInterfacePtr AplConfiguration::getMetricsRecorder() const {
    return m_metricsRecorder;
}

void AplConfiguration::setMetricsRecorder(Telemetry::AplMetricsRecorderInterfacePtr metricsRecorder) {
    if (metricsRecorder) {
        m_metricsRecorder = metricsRecorder;
    }
}

}
