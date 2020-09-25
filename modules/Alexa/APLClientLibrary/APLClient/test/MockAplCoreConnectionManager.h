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

#include <gtest/gtest.h>
#include <gmock/gmock.h>


namespace APLClient {
namespace test {
        class MockAplCoreConnectionManager : public AplCoreConnectionManager{
        public:
            MockAplCoreConnectionManager() : AplCoreConnectionManager(nullptr) {}
            MOCK_METHOD0(getScaleToViewhost, float());
            MOCK_METHOD0(aplCoreMetrics, std::shared_ptr<AplCoreMetrics>());
            MOCK_METHOD2(blockingSend, rapidjson::Document(AplCoreViewhostMessage& message,
                    const std::chrono::milliseconds& timeout));
            MOCK_METHOD1(setSupportedViewports, void(const std::string&));
            MOCK_METHOD2(setContent, void(const apl::ContentPtr, const std::string&));
        };
} // namespace test
} //namespace APLClient
