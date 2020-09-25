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

#include <string>

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTHANDLERINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTHANDLERINTERFACE_H

namespace APLClient {
namespace Extensions {

/**
 * The implementor of this interface should handle the call to invokeExtensionHandler on @c apl::RootContext
 * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/engine/rootcontext.h
 */
class AplCoreExtensionEventHandlerInterface {
public:
    virtual ~AplCoreExtensionEventHandlerInterface() = default;

    /**
     * Invoke an extension event handler handler
     * @param uri The URI of the custom document handler
     * @param name The name of the handler to invoke
     * @param data The data to associate with the handler
     * @param fastMode If true, this handler will be invoked in fast mode
     */
    virtual void invokeExtensionEventHandler(
        const std::string& uri,
        const std::string& name,
        const apl::ObjectMap& data,
        bool fastMode) = 0;
};

}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTHANDLERINTERFACE_H
