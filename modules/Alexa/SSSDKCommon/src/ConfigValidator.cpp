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

#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <SSSDKCommon/ConfigValidator.h>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

using namespace rapidjson;
using namespace std;

/// String to identify log entries originating from this file.
static const std::string TAG("ConfigValidator");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

static const std::string GUI_CONFIG_ROOT_KEY{"gui"};
static const std::string APPCONFIG_CONFIG_ROOT_KEY{"appConfig"};
static const std::string WINDOWS_CONFIG_ROOT_KEY{"windows"};
static const std::string DEVICEKEYS_CONFIG_ROOT_KEY{"deviceKeys"};
static const std::string VISUALCHARACTERISTICS_CONFIG_ROOT_KEY = "visualCharacteristics";

static const std::string APPCONFIG_DEFAULT_WINDOW_ID_KEY{"defaultWindowId"};
static const std::string WINDOWS_ID_KEY{"id"};
static const std::string WINDOWS_TEMPLATEID_KEY{"templateId"};
static const std::string WINDOWS_SIZECONFIGURATIONID_KEY{"sizeConfigurationId"};
static const std::string WINDOWS_INTERACTION_MODE_KEY{"interactionMode"};

static const std::string VISUALCHARACTERISTICS_INTERFACE_KEY{"interface"};

static const std::string ALEXADISPLAYWINDOW_INTERFACE_NAME{"Alexa.Display.Window"};
static const std::string ALEXADISPLAYWINDOW_CONFIGURATIONS_KEY{"configurations"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATES_KEY{"templates"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATE_ID_KEY{"id"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATE_CONFIGURATION_KEY{"configuration"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATE_SIZES_KEY{"sizes"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATE_SIZES_ID_KEY{"id"};
static const std::string ALEXADISPLAYWINDOW_TEMPLATE_INTERACTIONMODES_KEY{"interactionModes"};

static const std::string ALEXAINTERACTIONMODE_INTERFACE_NAME{"Alexa.InteractionMode"};
static const std::string ALEXAINTERACTIONMODE_CONFIGURATIONS_KEY{"configurations"};
static const std::string ALEXAINTERACTIONMODE_INTERFACTIONMODES_KEY{"interactionModes"};
static const std::string ALEXAINTERACTIONMODE_INTERFACTIONMODE_ID_KEY{"id"};

static const std::string APPCONFIG_DEVICEKEYS_KEYCODE_KEY{"keyCode"};
static const std::string APPCONFIG_DEVICEKEYS_KEYS[] =
    {"talkKey", "backKey", "exitKey", "toggleCaptionsKey", "toggleDoNotDisturbKey"};

ConfigValidator::ConfigValidator() {
}

std::shared_ptr<ConfigValidator> ConfigValidator::create() {
    std::shared_ptr<ConfigValidator> configValidator(new ConfigValidator());
    return configValidator;
}

bool ConfigValidator::validate(
    alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode& configuration,
    rapidjson::Document& jsonSchema) {
    rapidjson::SchemaDocument schema(jsonSchema);
    rapidjson::SchemaValidator validator(schema);
    rapidjson::Document doc;

    if (doc.Parse(configuration.serialize().c_str()).HasParseError()) {
        ACSDK_ERROR(LX(__func__).d("reason", "invalidConfigurationNode!"));
        return false;
    }

    // Validate configuration against schema
    if (!doc.Accept(validator)) {
        rapidjson::StringBuffer docBuffer, schemaBuffer;
        std::string validatorErrorMessage;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(schemaBuffer);
        validator.GetInvalidDocumentPointer().StringifyUriFragment(docBuffer);
        // Get the first error message
        validatorErrorMessage = std::string("configuration validation failed at ") + docBuffer.GetString() +
                                " against schema " + schemaBuffer.GetString() + " with keyword '" +
                                validator.GetInvalidSchemaKeyword() + "'";
        ACSDK_ERROR(LX(__func__).d("reason", "validationFailed").d("message", validatorErrorMessage));
        return false;
    }

    // Validate configuration against business logic
    auto guiConfig = configuration[GUI_CONFIG_ROOT_KEY];
    auto appConfig = guiConfig[APPCONFIG_CONFIG_ROOT_KEY];
    auto windowsConfig = appConfig.getArray(WINDOWS_CONFIG_ROOT_KEY);
    auto visualCharacteristicsConfig = guiConfig.getArray(VISUALCHARACTERISTICS_CONFIG_ROOT_KEY);
    auto deviceKeysConfig = appConfig[DEVICEKEYS_CONFIG_ROOT_KEY];

    // Extract gui app config default window id
    std::string defaultWindowId;
    appConfig.getString(APPCONFIG_DEFAULT_WINDOW_ID_KEY, &defaultWindowId);

    // Extract Alexa display window templates, Alexa interaction modes
    alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode alexaDisplayWindowInterface;
    alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode alexaInteractionModeInterface;
    for (size_t i = 0; i < visualCharacteristicsConfig.getArraySize(); i++) {
        std::string interfaceName;
        visualCharacteristicsConfig[i].getString(VISUALCHARACTERISTICS_INTERFACE_KEY, &interfaceName);
        if (interfaceName == ALEXADISPLAYWINDOW_INTERFACE_NAME) {
            alexaDisplayWindowInterface = visualCharacteristicsConfig[i];
        } else if (interfaceName == ALEXAINTERACTIONMODE_INTERFACE_NAME) {
            alexaInteractionModeInterface = visualCharacteristicsConfig[i];
        }
    }
    if (!alexaDisplayWindowInterface) {
        ACSDK_ERROR(LX(__func__).d("reason", "Alexa display window interface not found"));
        return false;
    }
    if (!alexaInteractionModeInterface) {
        ACSDK_ERROR(LX(__func__).d("reason", "Alexa interaction mode interface not found"));
        return false;
    }
    auto alexaDisplayWindowTemplates =
        alexaDisplayWindowInterface[ALEXADISPLAYWINDOW_CONFIGURATIONS_KEY].getArray(ALEXADISPLAYWINDOW_TEMPLATES_KEY);
    auto alexaInteractionModes = alexaInteractionModeInterface[ALEXADISPLAYWINDOW_CONFIGURATIONS_KEY].getArray(
        ALEXAINTERACTIONMODE_INTERFACTIONMODES_KEY);

    // Extract Alexa interaction mode ids;
    std::vector<std::string> alexaInteractionModeIds;
    for (size_t i = 0; i < alexaInteractionModes.getArraySize(); i++) {
        std::string alexaInteractionModeId;
        alexaInteractionModes[i].getString(ALEXAINTERACTIONMODE_INTERFACTIONMODE_ID_KEY, &alexaInteractionModeId);
        alexaInteractionModeIds.push_back(alexaInteractionModeId);
    }

    // Construct Alexa display window interface template map
    std::unordered_map<std::string, alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode>
        alexaDisplayWindowInterfaceTemplateMap;
    for (size_t i = 0; i < alexaDisplayWindowTemplates.getArraySize(); i++) {
        std::string alexaDisplayWindowTemplateId;
        alexaDisplayWindowTemplates[i].getString(ALEXADISPLAYWINDOW_TEMPLATE_ID_KEY, &alexaDisplayWindowTemplateId);
        alexaDisplayWindowInterfaceTemplateMap.insert({alexaDisplayWindowTemplateId, alexaDisplayWindowTemplates[i]});
    }

    // Validate Alexa display window interface templates interactionModes
    for (size_t i = 0; i < alexaDisplayWindowTemplates.getArraySize(); i++) {
        std::set<std::string> windowTemplateInteractionModes;
        alexaDisplayWindowTemplates[i][ALEXADISPLAYWINDOW_TEMPLATE_CONFIGURATION_KEY].getStringValues(
            ALEXADISPLAYWINDOW_TEMPLATE_INTERACTIONMODES_KEY, &windowTemplateInteractionModes);
        for (auto windowTemplateInteractionMode : windowTemplateInteractionModes) {
            if (std::find(
                    alexaInteractionModeIds.begin(), alexaInteractionModeIds.end(), windowTemplateInteractionMode) ==
                alexaInteractionModeIds.end()) {
                ACSDK_ERROR(LX(__func__)
                                .d("reason", "validationFailed")
                                .d("message", "InteractionModes ID not found in Alexa.InteractionMode interface"));
                return false;
            }
        }
    }

    // Gui app config should have valid default window id
    // Each gui app config window should have valid template id, size config id and interaction mode
    bool isDefaultWindowDefined = false;
    for (std::size_t i = 0; i < windowsConfig.getArraySize(); i++) {
        std::string windowId;
        std::string templateId;
        std::string sizeConfigurationId;
        std::string interactionMode;
        windowsConfig[i].getString(WINDOWS_ID_KEY, &windowId);
        windowsConfig[i].getString(WINDOWS_TEMPLATEID_KEY, &templateId);
        windowsConfig[i].getString(WINDOWS_SIZECONFIGURATIONID_KEY, &sizeConfigurationId);
        windowsConfig[i].getString(WINDOWS_INTERACTION_MODE_KEY, &interactionMode);

        if (windowId == defaultWindowId) {
            isDefaultWindowDefined = true;
        }

        // Extract target window template
        auto windowTemplate = alexaDisplayWindowInterfaceTemplateMap[templateId];
        if (!windowTemplate) {
            ACSDK_ERROR(LX(__func__).d("reason", "validationFailed").d("message", "Target window template not found"));
            return false;
        }

        // Validate sizeConfigurationId
        bool isSizeConfigurationDefined = false;
        auto windowTemplateSizes = windowTemplate[ALEXADISPLAYWINDOW_TEMPLATE_CONFIGURATION_KEY].getArray(
            ALEXADISPLAYWINDOW_TEMPLATE_SIZES_KEY);
        for (size_t i = 0; i < windowTemplateSizes.getArraySize(); i++) {
            std::string sizeId;
            windowTemplateSizes[i].getString(ALEXADISPLAYWINDOW_TEMPLATE_SIZES_ID_KEY, &sizeId);
            if (sizeId == sizeConfigurationId) {
                isSizeConfigurationDefined = true;
                break;
            }
        }
        if (!isSizeConfigurationDefined) {
            ACSDK_ERROR(LX(__func__)
                            .d("reason", "validationFailed")
                            .d("message", "Size configuration not found in target window template"));
            return false;
        }

        // Validate interactionMode
        std::set<std::string> windowTemplateInteractionModes;
        windowTemplate[ALEXADISPLAYWINDOW_TEMPLATE_CONFIGURATION_KEY].getStringValues(
            ALEXADISPLAYWINDOW_TEMPLATE_INTERACTIONMODES_KEY, &windowTemplateInteractionModes);
        if (windowTemplateInteractionModes.find(interactionMode) == windowTemplateInteractionModes.end()) {
            ACSDK_ERROR(LX(__func__)
                            .d("reason", "validationFailed")
                            .d("message", "Interaction mode not supported by target window template"));
            return false;
        }
    }
    if (!isDefaultWindowDefined) {
        ACSDK_ERROR(LX(__func__)
                        .d("reason", "validationFailed")
                        .d("message", "Default window not found in APL window configurations"));
        return false;
    }

    // Detect collision in gui app config device keys config
    std::unordered_map<int, std::string> deviceKeysKeyCodeMap;
    for (std::string deviceKey : APPCONFIG_DEVICEKEYS_KEYS) {
        int keyCode;
        deviceKeysConfig[deviceKey].getInt(APPCONFIG_DEVICEKEYS_KEYCODE_KEY, &keyCode);
        if (deviceKeysKeyCodeMap.find(keyCode) != deviceKeysKeyCodeMap.end()) {
            std::string keyCollisionMessage = deviceKeysKeyCodeMap[keyCode] + ", " + deviceKey;
            ACSDK_WARN(LX(__func__)
                           .d("reason", "validationIssueDetected")
                           .d("message", "Found collision in app config device key codes")
                           .d("keyCodes", keyCollisionMessage));
        } else {
            deviceKeysKeyCodeMap.insert({keyCode, deviceKey});
        }
    }

    return true;
}

};  // namespace sssdkCommon
};  // namespace alexaSmartScreenSDK
