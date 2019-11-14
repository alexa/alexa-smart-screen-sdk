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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "VisualCharacteristics/VisualCharacteristics.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace visualCharacteristics {

using namespace ::testing;
using namespace alexaClientSDK;
using namespace avsCommon;
using namespace avs;

class MockVisualCharacteristicsTest : public VisualCharacteristics {
public:
    MOCK_METHOD0(getVisualCharacteristicsCapabilityConfiguration, void());
};

/// Test harness for @c VisualCharacteristics class.
class VisualCharacteristicsTest : public ::testing::Test {
protected:
    /// A pointer to an instance of the VisualCharacteristics that will be instantiated per test.
    std::shared_ptr<StrictMock<MockVisualCharacteristicsTest>> m_visualCharacteristics;
};

/**
 * Tests that the VisualCharacteristics capability agent can successfully publish the four APIs in the config
 * file.
 */
TEST_F(VisualCharacteristicsTest, DISABLED_testGetCapabilityConfigurations) {
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        capabilityConfigurations = m_visualCharacteristics->getCapabilityConfigurations();

    EXPECT_CALL(*m_visualCharacteristics, getVisualCharacteristicsCapabilityConfiguration()).Times(Exactly(1));
    EXPECT_TRUE(capabilityConfigurations.size() == 4);
}

}  // namespace visualCharacteristics
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK