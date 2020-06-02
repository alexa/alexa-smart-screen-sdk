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

#ifndef ALEXA_SMART_SCREEN_SDK_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONSTATEMANAGER_H
#define ALEXA_SMART_SCREEN_SDK_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONSTATEMANAGER_H

#include <memory>
#include <AVSCommon/SDKInterfaces/Storage/MiscStorageInterface.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * This class manages if captions are turned on or off. It stores the status persistently
 * in disk
 */
class SmartScreenCaptionStateManager {
public:
    /**
     * Constructor
     * @param miscStorage the storage where Captions settings are stored
     */
    explicit SmartScreenCaptionStateManager(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> miscStorage);
    /**
     * Retrieves the current captions status
     * @return whether or not Captions are enabled
     */
    bool areCaptionsEnabled();
    /**
     * Toggles the current caption status
     */
    void toggleCaptions();

private:
    /// Pointer to the storage interface
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface> m_miscStorage;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_INCLUDE_SAMPLEAPP_SMARTSCREENCAPTIONSTATEMANAGER_H
