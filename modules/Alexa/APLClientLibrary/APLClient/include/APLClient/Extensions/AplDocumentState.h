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

#ifndef ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLDOCUMENTSTATE_H_
#define ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLDOCUMENTSTATE_H_

#include <memory>
#include <chrono>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma push_macro("DEBUG")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#undef DEBUG
#undef TRUE
#undef FALSE
#include <apl/apl.h>
#pragma pop_macro("DEBUG")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma GCC diagnostic pop

namespace APLClient {
namespace Extensions {

/**
 * The @c AplDocumentState is an object designed to cache the state of an active APL document such that it can
 * be re-inflated and restored.  i.e. as when used in Backstack navigation.
 */
struct AplDocumentState {
    /**
     * Default Constructor.
     */
    AplDocumentState() = default;

    /**
     * Constructor.
     * @param token The presentation token for the document.
     * @param rootContext The @c RootContext pointer for the document.
     * @param metrics The derived @c AplCoreMetrics for the document.
     */
    AplDocumentState(
        std::string token,
        apl::RootContextPtr rootContext,
        std::shared_ptr<apl::MetricsTransform> metrics) :
            token{std::move(token)},
            rootContext{std::move(rootContext)},
            metrics{std::move(metrics)} {};

    /// The id for the document state, as defined by the client or assigned in back navigation.
    std::string id;
    /// The presentationToken for the document as provided by the original APL document directive.
    std::string token;
    /// The pointer for the derived @c RootContext which maintains all state information about the document.
    apl::RootContextPtr rootContext;
    /// The pointer for the derived @c MetricsTransform for the document.
    std::shared_ptr<apl::MetricsTransform> metrics;
    /// The configuration change that needs to be applied to the restoring documentState.
    apl::ConfigurationChange configurationChange;
};

using AplDocumentStatePtr = std::shared_ptr<AplDocumentState>;

}  // namespace Extensions
}  // namespace APLClient
#endif  // ALEXA_SMART_SCREEN_SDK_APPLICATIONUTILITIES_APL_APLDOCUMENTSTATE_H_
