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

#ifndef APL_CLIENT_LIBRARY_DISPLAY_METRIC_H_
#define APL_CLIENT_LIBRARY_DISPLAY_METRIC_H_

#include <stdint.h>

#include <string>

namespace APLClient {

enum class DisplayMetricKind {
    kCounter,
    kTimer
};

struct DisplayMetric {
    DisplayMetricKind kind;
    std::string name;
    uint64_t value;
};

} // namespace APLClient

#endif // APL_CLIENT_LIBRARY_DISPLAY_METRIC_H_
