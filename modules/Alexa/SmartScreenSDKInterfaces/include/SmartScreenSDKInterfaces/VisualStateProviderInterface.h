/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_VISUALPROVIDERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_VISUALPROVIDERINTERFACE_H_

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * A @c VisualStateProviderInterface may be any client component that provides
 * updates on the visual context of the device.
 */
class VisualStateProviderInterface {
public:
    /**
     * Destructor.
     */
    virtual ~VisualStateProviderInterface() = default;

    /**
     * A request to a @c VisualStateProvider to provide the state.
     * The requester specifies a token which the @c VisualStateProvider must use when it updates
     * the state via the @c setState call to @c AlexaPresentation Capability Agent.
     *
     * @param stateRequestToken The token to use in the @c setState call.
     */
    virtual void provideState(const unsigned int stateRequestToken) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_VISUALPROVIDERINTERFACE_H_
