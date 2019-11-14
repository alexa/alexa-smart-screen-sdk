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

#ifndef ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKMESSAGESENDER_H_
#define ALEXA_CLIENT_SDK_AVSCOMMON_SDKINTERFACES_TEST_AVSCOMMON_SDKINTERFACES_MOCKMESSAGESENDER_H_

#include "AVSCommon/SDKInterfaces/MessageSenderInterface.h"
#include <gmock/gmock.h>

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {
namespace test {

/// Mock class that implements the MessageSender.
class MockMessageSender : public alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface {
public:
    MOCK_METHOD1(sendMessage, void(std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request));
};

}  // namespace test
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKMESSAGESENDER_H_
