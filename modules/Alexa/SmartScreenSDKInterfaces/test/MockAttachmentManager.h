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

#ifndef ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKATTACHMENTMANAGER_H_
#define ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKATTACHMENTMANAGER_H_

#include <chrono>
#include <string>
#include <memory>

#include <gmock/gmock.h>
#include "AVSCommon/AVS/Attachment/AttachmentManagerInterface.h"

namespace alexaSmartScreenSDK {
namespace smartScreenSDKInterfaces {
namespace test {

/// Mock class that implements the AttachmentManager.
class MockAttachmentManager : public alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface {
public:
    MOCK_CONST_METHOD2(generateAttachmentId, std::string(const std::string& contextId, const std::string& contentId));
    MOCK_METHOD1(setAttachmentTimeoutMinutes, bool(std::chrono::minutes timeoutMinutes));
    MOCK_METHOD2(
        createWriter,
        std::unique_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentWriter>(
            const std::string& attachmentId,
            alexaClientSDK::avsCommon::utils::sds::WriterPolicy policy));
    MOCK_METHOD2(
        createReader,
        std::unique_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader>(
            const std::string& attachmentId,
            alexaClientSDK::avsCommon::utils::sds::ReaderPolicy policy));
};

}  // namespace test
}  // namespace smartScreenSDKInterfaces
}  // namespace alexaSmartScreenSDK

#endif  // ALEXA_SMART_SCREEN_SDK_INTERFACES_TEST_MOCKATTACHMENTMANAGER_H_
