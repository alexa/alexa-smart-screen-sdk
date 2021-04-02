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

    return true;
}

};  // namespace sssdkCommon
};  // namespace alexaSmartScreenSDK
