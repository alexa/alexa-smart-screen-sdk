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

#include "APLClient/AplClientBinding.h"
#include "APLClient/AplClientRenderer.h"
#include "APLClient/AplCoreEngineLogBridge.h"
#include "APLClient/Telemetry/AplMetricsRecorder.h"
#include "APLClient/Telemetry/NullAplMetricsRecorder.h"

namespace APLClient {

AplClientBinding::AplClientBinding(AplOptionsInterfacePtr options) :
        m_aplConfiguration{std::make_shared<AplConfiguration>(options)} {
    apl::LoggerFactory::instance().initialize(std::make_shared<AplCoreEngineLogBridge>(options));
}

std::shared_ptr<AplClientRenderer> AplClientBinding::createRenderer(const std::string& windowId) {
    return std::make_shared<AplClientRenderer>(m_aplConfiguration, windowId);
}

Telemetry::DownloadMetricsEmitterPtr AplClientBinding::createDownloadMetricsEmitter() {
    return std::make_shared<Telemetry::DownloadMetricsEmitter>(m_aplConfiguration->getMetricsRecorder());
}

void AplClientBinding::onTelemetrySinkUpdated(APLClient::Telemetry::AplMetricsSinkInterfacePtr sink) {
    Telemetry::AplMetricsRecorderInterfacePtr recorder;
    if (sink) {
        recorder = Telemetry::AplMetricsRecorder::create(sink);
    } else {
        recorder = std::make_shared<Telemetry::NullAplMetricsRecorder>();
    }
    m_aplConfiguration->setMetricsRecorder(recorder);
}

}  // namespace APLClient