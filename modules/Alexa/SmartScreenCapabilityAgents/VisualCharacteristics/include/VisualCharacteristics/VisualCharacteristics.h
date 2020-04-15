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

#ifndef ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_VISUALCHARACTERISTICS_INCLUDE_VISUALCHARACTERISTICS_VISUALCHARACTERISTICS_H_
#define ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_VISUALCHARACTERISTICS_INCLUDE_VISUALCHARACTERISTICS_VISUALCHARACTERISTICS_H_

#include <chrono>
#include <memory>
#include <queue>
#include <string>
#include <unordered_set>

#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace visualCharacteristics {

/**
 * This class implements a @c CapabilityConfigurationInterface that publish viewport characteristic data.
 * This set of data includes all necessary information about its windows configuration. There are four APIs:
 * Alexa.Display: The display interface expresses explicitly the raw properties of a display.
 * Alexa.Display.Window: An expression of windows that may be created on a display.
 * Alexa.InteractionMode: Expression of interaction modes that the device intends to support.
 * Alexa.Presentation.APL.Video: Expression of the supported video codecs and playback abilities supported
 * by the device.
 */
class VisualCharacteristics
        : public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface
        , public std::enable_shared_from_this<VisualCharacteristics>
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    /**
     * Create an instance of @c VisualCharacteristics.
     * @param contextManager The @c ContextManagerInterface used to generate system context for events.
     */
    static std::shared_ptr<VisualCharacteristics> create(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /**
     * Destructor.
     */
    virtual ~VisualCharacteristics() = default;

    /// @name CapabilityConfigurationInterface Functions
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name StateProviderInterface Functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        unsigned int stateRequestToken) override;
    /// @}

    /**
     * Set device window state
     * @param deviceWindowState The input payload for device window state
     */
    void setDeviceWindowState(const std::string& deviceWindowState);

private:
    /**
     * Constructor.
     */
    VisualCharacteristics(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /**
     * Creates the VisualCharacteristics interface configuration.
     */
    void getVisualCharacteristicsCapabilityConfiguration();

    // @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// The @c ContextManager used to generate system context for events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The payload for device window state received from client
    std::string m_deviceWindowState;

    /// This is the worker thread for the @c VisualCharacteristics CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace visualCharacteristics
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_CAPABILITYAGENTS_VISUALCHARACTERISTICS_INCLUDE_VISUALCHARACTERISTICS_VISUALCHARACTERISTICS_H_
