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

#include "APLClient/Telemetry/AplMetricsRecorderInterface.h"

namespace APLClient {
namespace Telemetry {

const AplMetricsRecorderInterface::DocumentId AplMetricsRecorderInterface::UNKNOWN_DOCUMENT = 0;
const AplMetricsRecorderInterface::DocumentId AplMetricsRecorderInterface::CURRENT_DOCUMENT = 1;
const AplMetricsRecorderInterface::DocumentId AplMetricsRecorderInterface::LATEST_DOCUMENT = 2;

} // namespace Telemetry
} // namespace APLClient
