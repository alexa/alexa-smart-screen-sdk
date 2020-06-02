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

#include <ostream>
#include <fstream>

#include <rapidjson/error/en.h>

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>

#include "VisualCharacteristics/VisualCharacteristics.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace visualCharacteristics {

using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils;
using namespace alexaClientSDK::avsCommon::utils::configuration;
using namespace alexaClientSDK::avsCommon::utils::json;

/// String to identify log entries originating from this file.
static const std::string TAG{"VisualCharacteristics"};

/// The key in our config file to find the root of GUI configuration
static const std::string GUI_CONFIGURATION_ROOT_KEY = "gui";

/// The key in our config file to find the root of VisualCharacteristics configuration
static const std::string VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY = "visualCharacteristics";

/// The key in our config file to find the name of configuration node
static const std::string INTERFACE_CONFIGURATION_NAME_KEY = "interface";

/// The key in our config file to find the configurations of configuration node
static const std::string INTERFACE_CONFIGURATION_KEY = "configurations";

/// The default interface name if it's not present
static const std::string DEFAULT_INTERFACE_NAME = "";

/// Capability interface type
static const std::string CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Alexa.InteractionMode interface name
static const std::string ALEXAINTERACTIONMODE_CAPABILITY_INTERFACE_NAME = "Alexa.InteractionMode";
/// Alexa.InteractionMode interface version
static const std::string ALEXAINTERACTIONMODE_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.Presentation.APL.Video interface name
static const std::string ALEXAPRESENTATIONAPLVIDEO_CAPABILITY_INTERFACE_NAME = "Alexa.Presentation.APL.Video";
/// Alexa.Presentation.APL.Video interface version
static const std::string ALEXAPRESENTATIONAPLVIDEO_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.Display.Window interface name
static const std::string ALEXADISPLAYWINDOW_CAPABILITY_INTERFACE_NAME = "Alexa.Display.Window";
/// Alexa.Display.Window interface version
static const std::string ALEXADISPLAYWINDOW_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Alexa.Display interface name
static const std::string ALEXADISPLAY_CAPABILITY_INTERFACE_NAME = "Alexa.Display";
/// Alexa.Display interface version
static const std::string ALEXADISPLAY_CAPABILITY_INTERFACE_VERSION = "1.0";

/// Namespace three supported by Alexa presentation APL capability agent.
static const std::string ALEXA_DISPLAY_WINDOW_NAMESPACE{"Alexa.Display.Window"};

/// Tag for finding the device window state context information sent from the runtime as part of event context.
static const std::string WINDOW_STATE_NAME{"WindowState"};

/// The VisualCharacteristics context state signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName DEVICE_WINDOW_STATE{ALEXA_DISPLAY_WINDOW_NAMESPACE,
                                                                                  WINDOW_STATE_NAME};

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<VisualCharacteristics> VisualCharacteristics::create(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    if (!contextManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullcontextManager"));
        return nullptr;
    }

    std::shared_ptr<VisualCharacteristics> visualCharacteristics(new VisualCharacteristics(contextManager));
    contextManager->setStateProvider(DEVICE_WINDOW_STATE, visualCharacteristics);
    return visualCharacteristics;
}

void VisualCharacteristics::doShutdown() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.shutdown();
    m_contextManager.reset();
}

VisualCharacteristics::VisualCharacteristics(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) :
        RequiresShutdown{"VisualCharacteristics"} {
    m_contextManager = contextManager;

    getVisualCharacteristicsCapabilityConfiguration();
}

void VisualCharacteristics::getVisualCharacteristicsCapabilityConfiguration() {
    ACSDK_DEBUG9(LX(__func__));

    /// Get the root ConfigurationNode.
    auto configurationRoot = ConfigurationNode::getRoot();

    /// Get the root of GUI ConfigurationNode.
    auto configurationGui = configurationRoot[GUI_CONFIGURATION_ROOT_KEY];

    /// Get the ConfigurationNode contains VisualCharacteristics config array.
    auto configurationArray = configurationGui.getArray(VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY);

    /// Loop through the configuration node array and construct configMap for these APIs.
    for (size_t i = 0; i < configurationArray.getArraySize(); i++) {
        std::string interfaceName;
        configurationArray[i].getString(INTERFACE_CONFIGURATION_NAME_KEY, &interfaceName, DEFAULT_INTERFACE_NAME);

        if (ALEXAINTERACTIONMODE_CAPABILITY_INTERFACE_NAME == interfaceName) {
            std::unordered_map<std::string, std::string> configMap;
            std::string configurations = configurationArray[i][INTERFACE_CONFIGURATION_KEY].serialize();
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXAINTERACTIONMODE_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALEXAINTERACTIONMODE_CAPABILITY_INTERFACE_VERSION});
            configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations});
            m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));
        } else if (ALEXAPRESENTATIONAPLVIDEO_CAPABILITY_INTERFACE_NAME == interfaceName) {
            std::unordered_map<std::string, std::string> configMap;
            std::string configurations = configurationArray[i][INTERFACE_CONFIGURATION_KEY].serialize();
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXAPRESENTATIONAPLVIDEO_CAPABILITY_INTERFACE_NAME});
            configMap.insert(
                {CAPABILITY_INTERFACE_VERSION_KEY, ALEXAPRESENTATIONAPLVIDEO_CAPABILITY_INTERFACE_VERSION});
            configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations});
            m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));
        } else if (ALEXADISPLAYWINDOW_CAPABILITY_INTERFACE_NAME == interfaceName) {
            std::unordered_map<std::string, std::string> configMap;
            std::string configurations = configurationArray[i][INTERFACE_CONFIGURATION_KEY].serialize();
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXADISPLAYWINDOW_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALEXADISPLAYWINDOW_CAPABILITY_INTERFACE_VERSION});
            configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations});
            m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));
        } else if (ALEXADISPLAY_CAPABILITY_INTERFACE_NAME == interfaceName) {
            std::unordered_map<std::string, std::string> configMap;
            std::string configurations = configurationArray[i][INTERFACE_CONFIGURATION_KEY].serialize();
            configMap.insert({CAPABILITY_INTERFACE_TYPE_KEY, CAPABILITY_INTERFACE_TYPE});
            configMap.insert({CAPABILITY_INTERFACE_NAME_KEY, ALEXADISPLAY_CAPABILITY_INTERFACE_NAME});
            configMap.insert({CAPABILITY_INTERFACE_VERSION_KEY, ALEXADISPLAY_CAPABILITY_INTERFACE_VERSION});
            configMap.insert({CAPABILITY_INTERFACE_CONFIGURATIONS_KEY, configurations});
            m_capabilityConfigurations.insert(std::make_shared<CapabilityConfiguration>(configMap));
        }
    }
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> VisualCharacteristics::
    getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

void VisualCharacteristics::provideState(
    const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
    unsigned int stateRequestToken) {
    m_executor.submit([this, stateRequestToken] {
        m_contextManager->setState(
            DEVICE_WINDOW_STATE, m_deviceWindowState, StateRefreshPolicy::ALWAYS, stateRequestToken);
    });
}

void VisualCharacteristics::setDeviceWindowState(const std::string& deviceWindowState) {
    m_executor.submit([this, deviceWindowState] { m_deviceWindowState = deviceWindowState; });
}

}  // namespace visualCharacteristics
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
