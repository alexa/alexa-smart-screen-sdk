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

#ifndef ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MULTISESSIONSERVERINTERFACE_H_
#define ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MULTISESSIONSERVERINTERFACE_H_

#include <SmartScreenSDKInterfaces/MessageListenerInterface.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {

/**
 * Forward reference to Session
 */
class Session;

/**
 * An interface to a server that hosts multiple concurrent sessions
 */
class MultiSessionServerInterface : public MessageListenerInterface {
public:
    /**
     * Register a session with the server
     * @param session
     */
    virtual void addSession(std::shared_ptr<Session> session) = 0;

    /**
     * Un-register a session with the server
     * @param session
     */
    virtual void removeSession(std::shared_ptr<Session> session) = 0;
};

}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_SMARTSCREENSDKINTERFACES_INCLUDE_SMARTSCREENSDKINTERFACES_MULTISESSIONSERVERINTERFACE_H_
