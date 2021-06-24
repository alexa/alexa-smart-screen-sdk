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

#include "APLClient/Extensions/Backstack/AplBackstackExtension.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

namespace APLClient {
namespace Extensions {
namespace test {

static const std::string EXPECTED_URI = "aplext:backstack:10";
static const auto EXPECTED_EVENTSOURCE = std::make_shared<apl::ObjectMap>();
static const auto EXPECTED_EVENTPARAMS = std::make_shared<apl::ObjectMap>();

class MockAplBackstackExtensionObserverInterface : public Backstack::AplBackstackExtensionObserverInterface {
public:
    MOCK_METHOD1(onRestoreDocumentState, void(AplDocumentStatePtr));
};

class MockAplCoreExtensionEventCallbackResultInterface : public AplCoreExtensionEventCallbackResultInterface {
public:
    MOCK_METHOD2(onExtensionEventResult, void(unsigned int event, bool succeeded));
};

class AplBackstackExtensionTest : public ::testing::Test {
public:
    /// Set up the test harness for running a test.
    void SetUp() override;
    /// Clean up the test harness after running a test.
    void TearDown() override;

protected:
    /// Pointer to the @c AplBackstackExtension
    std::shared_ptr<Backstack::AplBackstackExtension> m_backstackExtension;
    std::shared_ptr<MockAplBackstackExtensionObserverInterface> m_backstackExtensionObserverInterface;
    std::shared_ptr<MockAplCoreExtensionEventCallbackResultInterface> m_aplCoreExtensionEventCallbackResultInterface;
    /// calling extensionEvent
    void extensionEvent(const std::string& commandName);
    /// reset event Params
    void resetEventParams(const std::list<std::string>& resetParams);
};

void AplBackstackExtensionTest::SetUp() {
    m_backstackExtensionObserverInterface = std::make_shared<NiceMock<MockAplBackstackExtensionObserverInterface>>();
    m_backstackExtension = std::make_shared<Backstack::AplBackstackExtension>(m_backstackExtensionObserverInterface);
    m_aplCoreExtensionEventCallbackResultInterface = std::make_shared<NiceMock<MockAplCoreExtensionEventCallbackResultInterface>>();
}

void AplBackstackExtensionTest::TearDown() {
    m_backstackExtension->reset();
}

void AplBackstackExtensionTest::extensionEvent(const std::string &commandName) {
    m_backstackExtension->onExtensionEvent(EXPECTED_URI,
                                           commandName,
                                           EXPECTED_EVENTSOURCE,
                                           EXPECTED_EVENTPARAMS,
                                           0,
                                           m_aplCoreExtensionEventCallbackResultInterface);
}

void AplBackstackExtensionTest::resetEventParams(const std::list<std::string>& resetParams) {
    for (auto& param : resetParams) {
        if (EXPECTED_EVENTPARAMS->count(param) == 1) {
            EXPECTED_EVENTPARAMS->erase(param);
        }
    }
    ASSERT_TRUE(EXPECTED_EVENTPARAMS->empty());
}

TEST_F(AplBackstackExtensionTest, GetUriSuccess) {
    ASSERT_EQ(EXPECTED_URI, m_backstackExtension->getUri());
}

TEST_F(AplBackstackExtensionTest, GetEnvironmentSuccess) {
    const apl::Object env = m_backstackExtension->getEnvironment();
    ASSERT_TRUE(env.get("backstack").getArray().empty());
}

TEST_F(AplBackstackExtensionTest, GetCommandDefinitionsSuccess) {
    const std::list<apl::ExtensionCommandDefinition> commandDefs = m_backstackExtension->getCommandDefinitions();
    ASSERT_TRUE(commandDefs.size() == 2);
    const apl::ExtensionCommandDefinition goBackCommand = commandDefs.front();
    ASSERT_EQ("GoBack", goBackCommand.getName());
    const apl::ExtensionCommandDefinition clearCommand = commandDefs.back();
    ASSERT_EQ("Clear", clearCommand.getName());
}

TEST_F(AplBackstackExtensionTest, GetEventHandlersSuccess) {
    const std::list<apl::ExtensionEventHandler> eventHandlers = m_backstackExtension->getEventHandlers();
    ASSERT_TRUE(eventHandlers.empty());
}

TEST_F(AplBackstackExtensionTest, ApplySettingsAndResetSuccess) {
    // before apply settings, the liveObjects contains nothing.
    const std::unordered_map<std::string, apl::LiveObjectPtr> liveData = m_backstackExtension->getLiveDataObjects();
    ASSERT_TRUE(liveData.empty());
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "Pager");
    m_backstackExtension->applySettings(settings);
    // then livedata contains expected itmes
    const std::unordered_map<std::string, apl::LiveObjectPtr> liveData2 = m_backstackExtension->getLiveDataObjects();
    ASSERT_FALSE(liveData2.empty());
    // and should cache active document
    ASSERT_TRUE(m_backstackExtension->shouldCacheActiveDocument());
    // and reset will reset everything
    m_backstackExtension->reset();
    ASSERT_FALSE(m_backstackExtension->shouldCacheActiveDocument());
}

TEST_F(AplBackstackExtensionTest, handleBackSuccess) {
    EXPECT_CALL(*m_backstackExtensionObserverInterface, onRestoreDocumentState(_)).Times(1);

    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "Pager");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("Pager", documentState->id);
    // handle back to previous doc success
    m_backstackExtension->setResponsibleForBackButton(false);
    ASSERT_TRUE(m_backstackExtension->handleBack());
}

TEST_F(AplBackstackExtensionTest, handleClearSuccess) {
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "Pager");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("Pager", documentState->id);
    // trigger clear operation, active document id should be empty
    m_backstackExtension->reset();
    ASSERT_FALSE(m_backstackExtension->shouldCacheActiveDocument());
}

TEST_F(AplBackstackExtensionTest, OnExtensionEventGoBackCountSuccess) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "Pager");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("Pager", documentState->id);
    // trigger extension event
    const std::string commandName = "GoBack";
    EXPECTED_EVENTPARAMS->emplace("backType", "count");
    EXPECTED_EVENTPARAMS->emplace("backValue", 1);
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"backType", "backValue"});
}

TEST_F(AplBackstackExtensionTest, OnExtensionEventGoBackIndexSuccess) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "Sequence");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("Sequence", documentState->id);
    // trigger extension event
    const std::string commandName = "GoBack";
    EXPECTED_EVENTPARAMS->emplace("backType", "index");
    EXPECTED_EVENTPARAMS->emplace("backValue", 0);
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"backType", "backValue"});
}

TEST_F(AplBackstackExtensionTest, OnExtensionEventGoBackIdSuccess) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, true)).Times(1);
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "ScrollView");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("ScrollView", documentState->id);
    // trigger extension event
    const std::string commandName = "GoBack";
    EXPECTED_EVENTPARAMS->emplace("backType", "id");
    EXPECTED_EVENTPARAMS->emplace("backValue", "ScrollView");
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"backType", "backValue"});
}

TEST_F(AplBackstackExtensionTest, OnExtensionEventGoBackIdFailure) {
    EXPECT_CALL(*m_aplCoreExtensionEventCallbackResultInterface, onExtensionEventResult(_, false)).Times(1);
    // apply settings
    auto settings = std::make_shared<apl::ObjectMap>();
    settings->emplace("backstackArrayName", "backstackIds");
    settings->emplace("backstackId", "ScrollView");
    m_backstackExtension->applySettings(settings);
    // add document to backStack
    auto documentState = std::make_shared<AplDocumentState>();
    m_backstackExtension->addDocumentStateToBackstack(documentState);
    ASSERT_EQ("ScrollView", documentState->id);
    // trigger extension event
    const std::string commandName = "GoBack";
    EXPECTED_EVENTPARAMS->emplace("backType", "id");
    EXPECTED_EVENTPARAMS->emplace("backValue", "Pager");
    extensionEvent(commandName);
    // reset event params
    resetEventParams({"backType", "backValue"});
}
}
}
}