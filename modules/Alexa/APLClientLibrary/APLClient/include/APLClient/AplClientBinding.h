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

#ifndef APL_CLIENT_LIBRARY_APL_CLIENT_BINDING_H_
#define APL_CLIENT_LIBRARY_APL_CLIENT_BINDING_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <rapidjson/document.h>

#include "AplConfiguration.h"
#include "AplCoreConnectionManager.h"
#include "AplCoreGuiRenderer.h"
#include "AplOptionsInterface.h"
#include "AplClientRenderer.h"
#include "Extensions/AplCoreExtensionInterface.h"
#include "Extensions/AplCoreExtensionEventCallbackResultInterface.h"
#include "Telemetry/DownloadMetricsEmitter.h"

namespace APLClient {

/**
 * AplClientBinding abstracts away many of the implementation details of integrating with the APLCoreEngine and exposes
 * a smaller interface to allow rendering of APL documents on a remote view host through a client provided IPC layer.
 */
class AplClientBinding {
public:
    /**
     * Constructor
     */
    AplClientBinding(AplOptionsInterfacePtr options);

    virtual ~AplClientBinding() = default;

    /**
     * Creates a new APL Client renderer object
     * @note Client Renderer is responsible for abstracting the interactions with APL Core Engine and must be
     * maintained per active window
     *
     * @param windowId unique id targeting for rendering APL from this context
     * @return a shared pointer of @c AplClientRenderer
     */
    std::shared_ptr<AplClientRenderer> createRenderer(const std::string& windowId);

    /**
     * Creates a new @c DownloadMetricsEmitter instance to monitor resource downloads.
     *
     * @return a shared pointer of @c DownloadMetricsEmitter already configured for telemetry.
     */
    Telemetry::DownloadMetricsEmitterPtr createDownloadMetricsEmitter();

    /**
     * Set an instance @c AplMetricsSinkInterface to the @c AplConfiguration
     *
     * @param a shared pointer of @c AplMetricsSinkInterface to be used.
     */
    void onTelemetrySinkUpdated(APLClient::Telemetry::AplMetricsSinkInterfacePtr sink);

private:
    AplConfigurationPtr m_aplConfiguration;
};
}  // namespace APLClient

#endif  // APL_CLIENT_LIBRARY_APL_CLIENT_BINDING_H_
