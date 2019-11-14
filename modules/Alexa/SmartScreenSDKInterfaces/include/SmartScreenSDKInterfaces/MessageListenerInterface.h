/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGELISTENERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGELISTENERINTERFACE_H_

#include <string>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * An interface that listens to incoming messages from arbitrary sources
 */
class MessageListenerInterface {
public:
    /**
     * Called when a new message is available on the arbitrary source channel
     *
     * @note Blocking in this handler will block delivery of further messages.
     * @param payload an arbitrary string
     */
    virtual void onMessage(const std::string& payload) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MESSAGELISTENERINTERFACE_H_
