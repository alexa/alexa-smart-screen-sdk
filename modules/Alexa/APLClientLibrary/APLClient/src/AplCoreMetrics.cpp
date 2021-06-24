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

#include "APLClient/AplCoreMetrics.h"

namespace APLClient {

static const float DEFAULT_BASE_DPI = 160.0f;

float AplCoreMetrics::toViewhost(float value) const {
    return value * getScaleToViewhost() * getDpi() / DEFAULT_BASE_DPI;
}

float AplCoreMetrics::toCore(float value) const {
    return value * getScaleToCore() * DEFAULT_BASE_DPI / getDpi();
}

float AplCoreMetrics::getViewhostWidth() const {
    return toViewhost(getMetrics().getWidth());
}

float AplCoreMetrics::getViewhostHeight() const {
    return toViewhost(getMetrics().getHeight());
}

float AplCoreMetrics::toCorePixel(float value) {
    return value * getScaleToCore();
}

}  // namespace APLClient