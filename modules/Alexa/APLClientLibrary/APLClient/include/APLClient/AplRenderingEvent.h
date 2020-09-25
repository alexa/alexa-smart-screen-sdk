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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_APLEVENT_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_APLEVENT_H_

namespace APLClient {

/// Enumeration of APL events that could be sent from GUI to @c AlexaPresentation.
enum class AplRenderingEvent {
    /// APL Core Engine started document inflation
    INFLATE_BEGIN,

    /// APL Core Engine started document inflation
    INFLATE_END,

    /// Text measure event was called
    TEXT_MEASURE,

    DOCUMENT_RENDERED,

    RENDER_ABORTED
};

}  // namespace APLClient

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_APLEVENT_H_
