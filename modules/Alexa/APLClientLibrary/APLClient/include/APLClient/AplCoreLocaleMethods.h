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

#ifndef APLCLIENTLIBRARY_APLCORELOCALEMETHODS_H
#define APLCLIENTLIBRARY_APLCORELOCALEMETHODS_H

#include "AplConfiguration.h"
#include "AplCoreConnectionManager.h"
#include "Telemetry/AplMetricsRecorderInterface.h"

namespace APLClient {

    class AplCoreLocaleMethods : public apl::LocaleMethods {
    public:
        /**
         * Constructor
         *
         * @param aplCoreConnectionManager Pointer to the APL Core connection manager
         */
        AplCoreLocaleMethods(
                AplCoreConnectionManagerPtr aplCoreConnectionManager,
                AplConfigurationPtr config);

        std::string toLowerCase(const std::string &value, const std::string &locale) override;
        std::string toUpperCase(const std::string &value, const std::string &locale) override;

    private:
        std::string toCase(const std::string &value, const std::string &locale, const std::string key);
        std::weak_ptr<AplCoreConnectionManager> m_aplCoreConnectionManager;

        AplConfigurationPtr m_aplConfiguration;
    };

}  // namespace APLClient
#endif //APLCLIENTLIBRARY_APLCORELOCALEMETHODS_H
