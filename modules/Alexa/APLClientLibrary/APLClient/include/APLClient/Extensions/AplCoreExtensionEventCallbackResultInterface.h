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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTCALLBACKRESULTINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTCALLBACKRESULTINTERFACE_H

namespace APLClient {
namespace Extensions {

/**
 * An interface for handling extension event results for ExtensionCommandDefinitions registered as requiring resolution.
 * https://github.com/alexa/apl-core-library/blob/master/aplcore/include/apl/content/extensioncommanddefinition.h
 */
class AplCoreExtensionEventCallbackResultInterface {
public:
    virtual ~AplCoreExtensionEventCallbackResultInterface() = default;

    /**
     * Callback function for the resolution of an @c AplCoreExtensionEvent
     * @param event Event id number.
     * @param succeeded Whether the event resolved successfully
     */
    virtual void onExtensionEventResult(unsigned int event, bool succeeded) = 0;
};

}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_APLCOREEXTENSIONEVENTCALLBACKRESULTINTERFACE_H
