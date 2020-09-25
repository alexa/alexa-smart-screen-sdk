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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENTIONEVENTCALLBACKINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENTIONEVENTCALLBACKINTERFACE_H

#include <string>
#include "AplCoreExtensionEventCallbackResultInterface.h"

namespace APLClient {
namespace Extensions {

/**
 * An interface for handling APL Extension events registered as ExtensionCommandDefinitions
 * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioncommanddefinition.h
 */
class AplCoreExtensionEventCallbackInterface {
public:
    virtual ~AplCoreExtensionEventCallbackInterface() = default;

    /**
     * Extension Event Callback function
     * @param uri Extension uri
     * @param name Extension event name
     * @param source Map of the source object that raised the event
     * @param params Map of the user-specified properties
     * @param event Event number
     * @param resultCallback Pointer to result callback interface
     */
    virtual void onExtensionEvent(
        const std::string& uri,
        const std::string& name,
        const apl::Object& source,
        const apl::Object& params,
        unsigned int event,
        std::shared_ptr<AplCoreExtensionEventCallbackResultInterface> resultCallback) = 0;
};

}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENTIONEVENTCALLBACKINTERFACE_H
