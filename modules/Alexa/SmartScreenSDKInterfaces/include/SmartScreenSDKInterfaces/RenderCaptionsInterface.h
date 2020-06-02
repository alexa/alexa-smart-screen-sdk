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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_RENDERCAPTIONSINTERFACE_H
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_RENDERCAPTIONSINTERFACE_H

#include <string>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * This class is used to render captions visually
 */
class RenderCaptionsInterface {
public:
    /**
     * Destructor
     */
    virtual ~RenderCaptionsInterface() = default;
    /**
     * Send captions to be rendered
     *
     * @param payload Captions to be rendered.
     */
    virtual void renderCaptions(const std::string& payload) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK
#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_RENDERCAPTIONSINTERFACE_H
