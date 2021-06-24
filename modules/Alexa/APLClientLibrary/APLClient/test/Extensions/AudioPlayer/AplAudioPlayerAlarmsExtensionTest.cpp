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

#include "APLClient/Extensions/AudioPlayer/AplAudioPlayerAlarmsExtension.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {
namespace test {

static const std::string EXPECTED_URI = "aplext:musicalarm:10";
static const auto EXPECTED_EVENTSOURCE = std::make_shared<apl::ObjectMap>();
static const auto EXPECTED_EVENTPARAMS = std::make_shared<apl::ObjectMap>();

class MockAplAudioPlayerAlarmsExtensionObserverInterface : public AudioPlayer::AplAudioPlayerAlarmsExtensionObserverInterface {
public:
    MOCK_METHOD0(onAudioPlayerAlarmDismiss, void());
    MOCK_METHOD0(onAudioPlayerAlarmSnooze, void());
};

class MockAplCoreExtensionEventCallbackResultInterface : public AplCoreExtensionEventCallbackResultInterface {
public:
    MOCK_METHOD2(onExtensionEventResult, void(unsigned int event, bool succeeded));
};

class AplAudioPlayerAlarmsExtensionTest : public ::testing::Test {
public:
    void SetUp() override;

protected:
    /// Pointer to the @c AplAudioPlayerAlarmsExtension
    std::shared_ptr<AudioPlayer::AplAudioPlayerAlarmsExtension> m_audioPlayerAlarmsExtension;
    std::shared_ptr<MockAplAudioPlayerAlarmsExtensionObserverInterface> m_audioPlayerAlarmsExtensionObserverInterface;
    std::shared_ptr<MockAplCoreExtensionEventCallbackResultInterface> m_aplCoreExtensionEventCallbackResultInterface;
};

void AplAudioPlayerAlarmsExtensionTest::SetUp() {
    m_aplCoreExtensionEventCallbackResultInterface = std::make_shared<NiceMock<MockAplCoreExtensionEventCallbackResultInterface>>();
    m_audioPlayerAlarmsExtensionObserverInterface = std::make_shared<NiceMock<MockAplAudioPlayerAlarmsExtensionObserverInterface>>();
    m_audioPlayerAlarmsExtension = std::make_shared<AudioPlayer::AplAudioPlayerAlarmsExtension>(m_audioPlayerAlarmsExtensionObserverInterface);
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, GetUriSuccess) {
    ASSERT_EQ(EXPECTED_URI, m_audioPlayerAlarmsExtension->getUri());
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, GetEnvironmentSuccess) {
    ASSERT_TRUE(m_audioPlayerAlarmsExtension->getEnvironment().empty());
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, GetCommandDefinitionsSuccess) {
    // expected FullSet Commands of audioPlayerAlarm extension
    auto expectedCommandSet = std::set<std::string>();
    expectedCommandSet.insert("DismissAlarm");
    expectedCommandSet.insert("SnoozeAlarm");
    // should have all command defined
    for (auto& command : m_audioPlayerAlarmsExtension->getCommandDefinitions()) {
        ASSERT_TRUE(expectedCommandSet.count(command.getName()) == 1);
        expectedCommandSet.erase(command.getName());
    }
    ASSERT_TRUE(expectedCommandSet.empty());
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, GetEventHandlersSuccess) {
    // no event handler for audioPlayerAlarm extension
    ASSERT_TRUE(m_audioPlayerAlarmsExtension->getEventHandlers().empty());
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, GetLiveDataObjectsSuccess) {
    // no Live data object for audioPlayerAlarm extension
    ASSERT_TRUE(m_audioPlayerAlarmsExtension->getLiveDataObjects().empty());
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, ApplySettingsSuccess) {
    // nothing should happen when applySettings
    auto settings = std::make_shared<apl::ObjectMap>();
    m_audioPlayerAlarmsExtension->applySettings(settings);
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, OnExtensionEventFailure) {
    // Given an invalid event to handle
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(1);
    const std::string commandName = "invalid";
    m_audioPlayerAlarmsExtension->onExtensionEvent(EXPECTED_URI,
                                                   commandName,
                                                   EXPECTED_EVENTSOURCE,
                                                   EXPECTED_EVENTPARAMS,
                                                   0,
                                                   m_aplCoreExtensionEventCallbackResultInterface);
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, OnExtensionEventDismissAlarmSuccess) {
    // Given an DismissAlarm event to handle
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    EXPECT_CALL(*m_audioPlayerAlarmsExtensionObserverInterface, onAudioPlayerAlarmDismiss()).Times(1);
    const std::string commandName = "DismissAlarm";
    m_audioPlayerAlarmsExtension->onExtensionEvent(EXPECTED_URI,
                                                   commandName,
                                                   EXPECTED_EVENTSOURCE,
                                                   EXPECTED_EVENTPARAMS,
                                                   0,
                                                   m_aplCoreExtensionEventCallbackResultInterface);
}

TEST_F(AplAudioPlayerAlarmsExtensionTest, OnExtensionEventSnoozeAlarmSuccess) {
    // Given an SnoozeAlarm event to handle
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    EXPECT_CALL(*m_audioPlayerAlarmsExtensionObserverInterface, onAudioPlayerAlarmSnooze()).Times(1);
    const std::string commandName = "SnoozeAlarm";
    m_audioPlayerAlarmsExtension->onExtensionEvent(EXPECTED_URI,
                                                   commandName,
                                                   EXPECTED_EVENTSOURCE,
                                                   EXPECTED_EVENTPARAMS,
                                                   0,
                                                   m_aplCoreExtensionEventCallbackResultInterface);
}

}
}
}
}