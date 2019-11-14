/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORETEXTMEASUREMENT_H
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORETEXTMEASUREMENT_H

// TODO: Tidy up core to prevent this (ARC-917)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/apl.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop

#include "AplCoreConnectionManager.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

class AplCoreTextMeasurement : public apl::TextMeasurement {
public:
    /**
     * Constructor
     *
     * @param aplCoreConnectionManager Pointer to the APL Core connection manager
     */
    AplCoreTextMeasurement(const std::shared_ptr<AplCoreConnectionManager> aplCoreConnectionManager) :
            m_AplCoreConnectionManager(aplCoreConnectionManager) {
    }

    /// @name apl::TextMeasurement Functions
    /// @{
    virtual YGSize measure(
        apl::TextComponent* component,
        float width,
        YGMeasureMode widthMode,
        float height,
        YGMeasureMode heightMode) override;

    virtual float baseline(apl::TextComponent* component, float width, float height) override;
    /// @}

private:
    std::shared_ptr<AplCoreConnectionManager> m_AplCoreConnectionManager;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_APLCORETEXTMEASUREMENT_H
