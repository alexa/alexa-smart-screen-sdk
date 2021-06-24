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

#include <climits>

#include "APLClient/AplCoreViewhostMessage.h"
#include "APLClient/AplCoreLocaleMethods.h"

namespace APLClient {
    static const char LOCALE_METHODS_KEY[] = "localeMethod";
    static const char UPPER_KEY[] = "toUpperCase";
    static const char LOWER_KEY[] = "toLowerCase";

    AplCoreLocaleMethods::AplCoreLocaleMethods(
            AplCoreConnectionManagerPtr aplCoreConnectionManager,
            AplConfigurationPtr config)
            : m_aplCoreConnectionManager{aplCoreConnectionManager},
              m_aplConfiguration{config} {
    }

    std::string AplCoreLocaleMethods::toUpperCase(const std::string &value, const std::string &locale) {
        return toCase(value, locale, UPPER_KEY);
    }
    std::string AplCoreLocaleMethods::toLowerCase(const std::string &value, const std::string &locale) {
        return toCase(value, locale, LOWER_KEY);
    }

    std::string AplCoreLocaleMethods::toCase(const std::string &value, const std::string &locale, const std::string methodName) {
        auto aplOptions = m_aplConfiguration->getAplOptions();
        if (auto aplCoreConnectionManager = m_aplCoreConnectionManager.lock()) {
            auto msg = AplCoreViewhostMessage(LOCALE_METHODS_KEY);
            auto& alloc = msg.alloc();

            rapidjson::Value payload(rapidjson::kObjectType);
            payload.AddMember("method", methodName, alloc);
            payload.AddMember("value", value, alloc);
            payload.AddMember("locale", locale, alloc);
            msg.setPayload(std::move(payload));

            auto result = aplCoreConnectionManager->blockingSend(msg);

            if (result.IsObject()) {
                auto casedValue = result["payload"]["value"].GetString();
                return casedValue;
            }

            aplOptions->logMessage(LogLevel::WARN, __func__, "Didn't get a valid reply.  Returning unlocalized value.");
            return value;
        } else {
            aplOptions->logMessage(LogLevel::WARN, __func__, "ConnectionManager does not exist. Returning unlocalized value");
            return value;
        }
    }

}  // namespace APLClient
