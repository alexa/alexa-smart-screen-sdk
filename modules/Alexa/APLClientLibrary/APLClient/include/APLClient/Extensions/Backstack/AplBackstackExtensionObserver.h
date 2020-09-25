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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSIONOBSERVERINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSIONOBSERVERINTERFACE_H

#include "APLClient/Extensions/AplDocumentState.h"

namespace APLClient {
namespace Extensions {
namespace Backstack {

/**
 * This class allows a @c AplBackstackExtensionObserverInterface observer to be notified of changes in the
 * @c AplBackstackExtension.
 */
class AplBackstackExtensionObserverInterface {
public:
    /**
     * Destructor
     */
    virtual ~AplBackstackExtensionObserverInterface() = default;

    /**
     * Used to notify the observer when the extension has successfully popped a @c AplDocumentState to be restored
     * from the @c AplBackstack.
     * @param documentState The popped @c AplDocumentState to restore.
     */
    virtual void onRestoreDocumentState(AplDocumentStatePtr documentState) = 0;
};

}  // namespace Backstack
}  // namespace Extensions
}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_EXTENSIONS_BACKSTACK_APLBACKSTACKEXTENSIONOBSERVERINTERFACE_H
