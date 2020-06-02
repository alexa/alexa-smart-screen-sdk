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

#ifndef ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKDIRECTIVEHANDLERRESULT_H_
#define ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKDIRECTIVEHANDLERRESULT_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "AVSCommon/SDKInterfaces/DirectiveHandlerResultInterface.h"

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {
namespace test {

/// Mock @c DirectiveHandlerResultInterface implementation.
class MockDirectiveHandlerResult : public alexaClientSDK::avsCommon::sdkInterfaces::DirectiveHandlerResultInterface {
public:
    MOCK_METHOD0(setCompleted, void());
    MOCK_METHOD1(setFailed, void(const std::string& description));
};

}  // namespace test
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKDIRECTIVEHANDLERRESULT_H_
