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

#include "APLClient/Extensions/AudioPlayer/AplAudioPlayerExtension.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

namespace APLClient {
namespace Extensions {
namespace AudioPlayer {
namespace test {

static const std::string SKILL_ID = "testSkill";
static const std::string EXPECTED_URI = "aplext:audioplayer:10";
static const std::string EXPECTED_TOKEN = "testingLyricsToken";
static const auto EXPECTED_EVENTSOURCE = std::make_shared<apl::ObjectMap>();
static const auto EXPECTED_EVENTPARAMS = std::make_shared<apl::ObjectMap>();

class MockAplAudioPlayerExtensionObserverInterface : public AudioPlayer::AplAudioPlayerExtensionObserverInterface {
public:
    MOCK_METHOD0(onAudioPlayerPlay, void());
    MOCK_METHOD0(onAudioPlayerPause, void());
    MOCK_METHOD0(onAudioPlayerNext, void());
    MOCK_METHOD0(onAudioPlayerPrevious, void());
    MOCK_METHOD1(onAudioPlayerSeekToPosition, void(int offsetInMilliseconds));
    MOCK_METHOD2(onAudioPlayerToggle, void(const std::string& name, bool checked));
    MOCK_METHOD3(onAudioPlayerLyricDataFlushed, void(const std::string& token, long durationInMilliseconds,  const std::string& lyricData));
    MOCK_METHOD0(onAudioPlayerSkipForward, void());
    MOCK_METHOD0(onAudioPlayerSkipBackward, void());
};

class MockAplCoreExtensionEventCallbackResultInterface : public AplCoreExtensionEventCallbackResultInterface {
public:
    MOCK_METHOD2(onExtensionEventResult, void(unsigned int event, bool succeeded));
};

class AplAudioPlayerExtensionTest : public ::testing::Test {
public:
    /// Set up the test harness for running a test.
    void SetUp() override;

    /// Clean up the test harness after running a test.
    void TearDown() override;

protected:
    /// Pointer to the @c AplAudioPlayerExtension
    std::shared_ptr<AudioPlayer::AplAudioPlayerExtension> m_audioPlayerExtension;
    std::shared_ptr<MockAplAudioPlayerExtensionObserverInterface> m_audioPlayerExtensionObserverInterface;
    std::shared_ptr<MockAplCoreExtensionEventCallbackResultInterface> m_aplCoreExtensionEventCallbackResultInterface;
    /// calling extensionEvent
    void extensionEvent(const std::string& commandName);
    /// reset event Params
    void resetEventParams(const std::list<std::string>& resetParams);
};

void AplAudioPlayerExtensionTest::SetUp() {
    m_aplCoreExtensionEventCallbackResultInterface = std::make_shared<NiceMock<MockAplCoreExtensionEventCallbackResultInterface>>();
    m_audioPlayerExtensionObserverInterface = std::make_shared<NiceMock<MockAplAudioPlayerExtensionObserverInterface>>();
    m_audioPlayerExtension = std::make_shared<AudioPlayer::AplAudioPlayerExtension>(m_audioPlayerExtensionObserverInterface);
    m_audioPlayerExtension->setActivePresentationSession(SKILL_ID, SKILL_ID);
}

void AplAudioPlayerExtensionTest::TearDown() {
}

void AplAudioPlayerExtensionTest::extensionEvent(const std::string& commandName) {
    m_audioPlayerExtension->onExtensionEvent(EXPECTED_URI,
                                             commandName,
                                             EXPECTED_EVENTSOURCE,
                                             EXPECTED_EVENTPARAMS,
                                             0,
                                             m_aplCoreExtensionEventCallbackResultInterface);
}

void AplAudioPlayerExtensionTest::resetEventParams(const std::list<std::string>& resetParams) {
    for (auto& param : resetParams) {
        if (EXPECTED_EVENTPARAMS->count(param) == 1) {
            EXPECTED_EVENTPARAMS->erase(param);
        }
    }
    ASSERT_TRUE(EXPECTED_EVENTPARAMS->empty());
}

TEST_F(AplAudioPlayerExtensionTest, GetUriSuccess) {
    ASSERT_EQ(EXPECTED_URI, m_audioPlayerExtension->getUri());
}

TEST_F(AplAudioPlayerExtensionTest, GetEnvironmentSuccess) {
    // audioPlayer extension does not have environments
    ASSERT_TRUE(m_audioPlayerExtension->getEnvironment().empty());
}

TEST_F(AplAudioPlayerExtensionTest, GetCommandDefinitionsSuccess) {
    // FullSet Commands for audio player
    auto expectedCommandSet = std::set<std::string>();
    expectedCommandSet.insert("Play");
    expectedCommandSet.insert("Pause");
    expectedCommandSet.insert("Previous");
    expectedCommandSet.insert("Next");
    expectedCommandSet.insert("SeekToPosition");
    expectedCommandSet.insert("Toggle");
    expectedCommandSet.insert("AddLyricsViewed");
    expectedCommandSet.insert("AddLyricsDurationInMilliseconds");
    expectedCommandSet.insert("FlushLyricData");
    expectedCommandSet.insert("SkipForward");
    expectedCommandSet.insert("SkipBackward");
    // should have all command defined
    for (auto& command : m_audioPlayerExtension->getCommandDefinitions()) {
        ASSERT_TRUE(expectedCommandSet.count(command.getName()) == 1);
        expectedCommandSet.erase(command.getName());
    }
    ASSERT_TRUE(expectedCommandSet.empty());
}

TEST_F(AplAudioPlayerExtensionTest, GetEventHandlersSuccess) {
    // FullSet event handler for audio player
    auto expectedHandlerSet = std::set<std::string>();
    expectedHandlerSet.insert("OnPlayerActivityUpdated");
    // should have all event handlers defined
    for (auto& handler : m_audioPlayerExtension->getEventHandlers()) {
        ASSERT_TRUE(expectedHandlerSet.count(handler.getName()) == 1);
        expectedHandlerSet.erase(handler.getName());
    }
    ASSERT_TRUE(expectedHandlerSet.empty());
}

TEST_F(AplAudioPlayerExtensionTest, GetLiveDataObjectsSuccess) {
    // before applySettings();
    auto liveObjects = m_audioPlayerExtension->getLiveDataObjects();
    // should have no liveObjects
    ASSERT_TRUE(liveObjects.empty());
    // with applySettings
    std::string expectedStateName = "unitTest";
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("playbackStateName", expectedStateName);
    m_audioPlayerExtension->applySettings(settings);
    // should have liveObjects with expected stateName defined
    liveObjects = m_audioPlayerExtension->getLiveDataObjects();
    ASSERT_TRUE(liveObjects.count(expectedStateName) == 1);
    apl::LiveMap* playbackState = dynamic_cast<apl::LiveMap*>(liveObjects.find(expectedStateName)->second.get());
    ASSERT_EQ(apl::Object::ObjectType::kMapType, playbackState->getType());
    ASSERT_EQ("STOPPED", playbackState->get("playerActivity").asString());
    ASSERT_EQ(0, playbackState->get("offset").asInt());
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventPlaySuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerPlay()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "Play";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventPauseSuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerPause()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "Pause";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventPreviousSuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerPrevious()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "Previous";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventNextSuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerNext()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "Next";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventSeekToPositionFailure) {
    // the observer don't be called due to no offset in event params
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerSeekToPosition(_)).Times(0);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(1);
    const std::string commandName = "SeekToPosition";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventSeekToPositionSuccess) {
    const int expectedOffset = 5;
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerSeekToPosition(expectedOffset)).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    EXPECTED_EVENTPARAMS->emplace("offset", expectedOffset);
    const std::string commandName = "SeekToPosition";
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"offset"});
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventSkipForwardSuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerSkipForward()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "SkipForward";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventSkipBackwardSuccess) {
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerSkipBackward()).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "SkipBackward";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventToggleFailure) {
    // the observer don't be called due to no toggle name in event params
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerToggle(_, _)).Times(0);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(2);
    const std::string commandName = "Toggle";
    extensionEvent(commandName);

    // or the toggleName is not valid
    const std::string invalidToggleName = "Invalid";
    EXPECTED_EVENTPARAMS->emplace("name", invalidToggleName);
    EXPECTED_EVENTPARAMS->emplace("checked", true);
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"name", "checked"});
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventToggleSuccess) {
    const std::string toggleName = "shuffle";
    const bool toggleChecked = true;
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface, onAudioPlayerToggle(toggleName, toggleChecked)).Times(1);
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "Toggle";
    EXPECTED_EVENTPARAMS->emplace("name", toggleName);
    EXPECTED_EVENTPARAMS->emplace("checked", toggleChecked);
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"name", "checked"});
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventAddLyricsViewedFailure) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(1);
    const std::string commandName = "AddLyricsViewed";
    extensionEvent(commandName);
}

MATCHER_P(LyricNotContainKey, key, "") { return arg.find(key) == std::string::npos; }

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventAddLyricsViewedAndFlushLyricsSuccess) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(2);
    const std::string commandName = "AddLyricsViewed";
    const std::string invalidLyricProperty = "invalid";
    auto line = std::make_shared<apl::ObjectMap>();
    line->emplace("text", "lyrics");
    // invalid lyric property
    line->emplace(invalidLyricProperty, "should ignore");
    auto lines = std::make_shared<apl::ObjectArray>();
    lines->emplace_back(line);
    EXPECTED_EVENTPARAMS->emplace("token", EXPECTED_TOKEN);
    EXPECTED_EVENTPARAMS->emplace("lines", lines);
    extensionEvent(commandName);

    // Flush Lyrics Data
    // invalid lyric property should not been added to lyricsData
    EXPECT_CALL(*m_audioPlayerExtensionObserverInterface,
                onAudioPlayerLyricDataFlushed(EXPECTED_TOKEN, _, LyricNotContainKey(invalidLyricProperty))).Times(1);
    const std::string flushCommand = "FlushLyricData";
    extensionEvent(flushCommand);

    // reset event params
    resetEventParams({"token", "lines"});
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventAddLyricsDurationInMillisecondsFailure) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(1);
    const std::string commandName = "AddLyricsDurationInMilliseconds";
    extensionEvent(commandName);
}

TEST_F(AplAudioPlayerExtensionTest, OnExtensionEventAddLyricsDurationInMillisecondsSuccess) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    const std::string commandName = "AddLyricsDurationInMilliseconds";

    EXPECTED_EVENTPARAMS->emplace("token", EXPECTED_TOKEN);
    EXPECTED_EVENTPARAMS->emplace("durationInMilliseconds", 0);
    extensionEvent(commandName);

    // reset event params
    resetEventParams({"token", "durationInMilliseconds"});
}

TEST_F(AplAudioPlayerExtensionTest,UpdatePlaybackProgressSuccess) {
    // applySettings
    std::string expectedStateName = "unitTest";
    const int expectedOffset = 100;
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("playbackStateName", expectedStateName);
    m_audioPlayerExtension->applySettings(settings);
    // update playback progress in playbackState
    m_audioPlayerExtension->updatePlaybackProgress(expectedOffset);
    // should have liveObjects with expected stateName defined
    auto liveObjects = m_audioPlayerExtension->getLiveDataObjects();
    ASSERT_TRUE(liveObjects.count(expectedStateName) == 1);
    apl::LiveMap* playbackState = dynamic_cast<apl::LiveMap*>(liveObjects.find(expectedStateName)->second.get());
    ASSERT_EQ(expectedOffset, playbackState->get("offset").asInt());
}

TEST_F(AplAudioPlayerExtensionTest,UpdatePlayerActivitySuccess) {
    // applySettings
    std::string expectedStateName = "unitTest";
    const int expectedOffset = 100;
    std::string expectedPlayerActivity = "PLAYING";
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("playbackStateName", expectedStateName);
    m_audioPlayerExtension->applySettings(settings);
    // update player activity in playbackState
    m_audioPlayerExtension->updatePlayerActivity(expectedPlayerActivity, expectedOffset);

    // should have liveObjects with expected stateName defined
    auto liveObjects = m_audioPlayerExtension->getLiveDataObjects();
    ASSERT_TRUE(liveObjects.count(expectedStateName) == 1);
    apl::LiveMap* playbackState = dynamic_cast<apl::LiveMap*>(liveObjects.find(expectedStateName)->second.get());
    ASSERT_EQ(expectedOffset, playbackState->get("offset").asInt());
    ASSERT_EQ(expectedPlayerActivity, playbackState->get("playerActivity").asString());
}

TEST_F(AplAudioPlayerExtensionTest,UpdatePlayerActivityFailure) {
    // applySettings
    std::string expectedStateName = "unitTest";
    std::string invalidPlayerActivity = "Invalid";
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("playbackStateName", expectedStateName);
    m_audioPlayerExtension->applySettings(settings);
    // update player activity with invalid activity
    m_audioPlayerExtension->updatePlayerActivity(invalidPlayerActivity, 0);

    // should not set the player activity in playerState
    auto liveObjects = m_audioPlayerExtension->getLiveDataObjects();
    ASSERT_TRUE(liveObjects.count(expectedStateName) == 1);
    apl::LiveMap* playbackState = dynamic_cast<apl::LiveMap*>(liveObjects.find(expectedStateName)->second.get());
    // the player activity is not set.
    ASSERT_EQ("STOPPED", playbackState->get("playerActivity").asString());
}

}
}
}
}