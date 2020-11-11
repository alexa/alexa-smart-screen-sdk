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

#ifndef ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DONOTDISTURBSETTINGOBSERVER_H_
#define ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DONOTDISTURBSETTINGOBSERVER_H_

#include <Settings/SettingObserverInterface.h>
#include <Settings/SettingsManager.h>
#include <Settings/DeviceSettingsManager.h>

namespace alexaSmartScreenSDK {
namespace sampleApp {

/**
 * This class is used to observe DND Setting changes
 */
class DoNotDisturbSettingObserver {
public:
    /**
     * Destructor
     */
    virtual ~DoNotDisturbSettingObserver() = default;

    /**
     * This function will be called when there's a change in the DoNotDisturb Setting
     *
     * @param enable The DoNotDisturb setting
     */
    virtual void onDoNotDisturbSettingChanged(bool enable) = 0;
};

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
#endif  // ALEXA_SMART_SCREEN_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_DONOTDISTURBSETTINGOBSERVER_H_
