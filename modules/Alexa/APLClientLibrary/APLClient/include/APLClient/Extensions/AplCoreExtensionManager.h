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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONMANAGER_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONMANAGER_H

#include <map>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/content/rootconfig.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop
#include "AplCoreExtensionInterface.h"

namespace APLClient {
namespace Extensions {

/**
 * A utility manager for tracking and registering supported @c AplCoreExtensionInterfaces with
 * instances of @c apl::RootConfig.
 *
 * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-extensions.html
 */
class AplCoreExtensionManager : public AplCoreExtensionEventCallbackInterface {
public:
    /**
     * Constructor
     */
    ~AplCoreExtensionManager() override = default;

    /**
     * Gets an Extension by its uri
     * @param uri Extension Uri
     * @return Shared Pointer to @c AplCoreExtensionInterface
     */
    std::shared_ptr<AplCoreExtensionInterface> getExtension(const std::string& uri);

    /**
     * Adds an Extension to the manager
     * @param extension Shared Pointer to @c AplCoreExtensionInterface
     */
    void addExtension(std::shared_ptr<AplCoreExtensionInterface> extension);

    /**
     * Registers a managed extension with the provided @c RootConfig
     * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/rootconfig.h
     * @param uri Uri of Extension to register
     * @param config RootConfig of the APL Document
     */
    void registerRequestedExtension(const std::string& uri, apl::RootConfig& config);

    /// @name AplCoreExtensionEventCallbackInterface Functions
    /// @{
    void onExtensionEvent(
        const std::string& uri,
        const std::string& name,
        const apl::Object& source,
        const apl::Object& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback = nullptr) override;
    /// @}

private:
    /// Map of @c AplCoreExtensionInterfaces by uri
    std::unordered_map<std::string, std::shared_ptr<AplCoreExtensionInterface>> m_Extensions;
};

using AplCoreExtensionManagerPtr = std::shared_ptr<AplCoreExtensionManager>;

}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONMANAGER_H
