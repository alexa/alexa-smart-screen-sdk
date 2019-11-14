/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <condition_variable>
#include <future>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rapidjson/document.h>

#include "MockAttachmentManager.h"
#include "MockContextManager.h"
#include "MockDirectiveHandlerResult.h"
#include "MockExceptionEncounteredSender.h"
#include "MockMessageSender.h"
#include "MockFocusManager.h"
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Logger/Logger.h>
#include <AVSCommon/Utils/Memory/Memory.h>

#include <SmartScreenSDKInterfaces/ActivityEvent.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/VisualStateProviderInterface.h>

#include "AlexaPresentation/AlexaPresentation.h"

namespace alexaSmartScreenSDK {
namespace smartScreenCapabilityAgents {
namespace alexaPresentation {
namespace test {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::configuration;
using namespace alexaClientSDK::avsCommon::utils::memory;
using namespace rapidjson;
using namespace ::testing;

/// Alias for JSON stream type used in @c ConfigurationNode initialization
using JSONStream = std::vector<std::shared_ptr<std::istream>>;

/// Timeout when waiting for futures to be set.
static std::chrono::milliseconds TIMEOUT(1000);

/// The second namespace registered for this capability agent.
static const std::string NAMESPACE1{"Alexa.Presentation"};

/// The third namespace registered for this capability agent.
static const std::string NAMESPACE2{"Alexa.Presentation.APL"};

/// An unknown directive signature.
static const std::string UNKNOWN_DIRECTIVE{"Unknown"};

/// The RenderDocument directive signature.
static const NamespaceAndName DOCUMENT{NAMESPACE2, "RenderDocument"};

/// The RenderDocument directive signature.
static const NamespaceAndName COMMAND{NAMESPACE2, "ExecuteCommands"};

/// The name for UserEvent event.
static const std::string USER_EVENT_EVENT{"UserEvent"};

/// The name for UserEvent event.
static const std::string DOCUMENT_DISMISSED_EVENT{"Dismissed"};

/// The @c MessageId identifer.
static const std::string MESSAGE_ID("messageId");

/// The @c MessageId identifer.
static const std::string MESSAGE_ID_2("messageId2");

/// Payload to be sent for UserEvent
static const std::string SAMPLE_USER_EVENT_PAYLOAD = R"({"key":"value"})";

/// String to identify log entries originating from this file.
static const std::string TAG("AlexaPresentationTest");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/**
 * Expected payload to be sent with Dismissed event when
 * @c DOCUMENT_APL_PAYLOAD was the RenderDocument directive payload
 */
static const std::string EXPECTED_DOCUMENT_DISMISSED_PAYLOAD = R"({"presentationToken":"APL_TOKEN"})";

/**
 * Expected payload to be sent with Dismissed event when
 * @c DOCUMENT_APL_PAYLOAD_2 was the RenderDocument directive payload
 */
static const std::string EXPECTED_DOCUMENT_DISMISSED_PAYLOAD_2 = R"({"presentationToken":"APL_TOKEN_2"})";

/// JSON key for the event section of a message.
static const std::string MESSAGE_EVENT_KEY = "event";

/// JSON key for the namespace field of a message header.
static const std::string MESSAGE_NAMESPACE_KEY = "namespace";

/// JSON key for the name field of a message header.
static const std::string MESSAGE_NAME_KEY = "name";

/// JSON key for the header section of a message.
static const std::string MESSAGE_HEADER_KEY = "header";

/// JSON key for the payload section of an message.
static const std::string MESSAGE_PAYLOAD_KEY = "payload";

// clang-format off

/// A RenderDocument directive with APL payload.
 static const std::string DOCUMENT_APL_PAYLOAD = "{"
                                                "\"presentationToken\":\"APL_TOKEN\","
                                                "\"windowId\":\"WINDOW_ID\","
                                                "\"document\":\"{}\""
                                                "}";

/// A 2nd RenderDocument directive with APL payload.
 static const std::string DOCUMENT_APL_PAYLOAD_2 = "{"
                                                  "\"presentationToken\":\"APL_TOKEN_2\","
                                                  "\"windowId\":\"WINDOW_ID\","
                                                  "\"document\":\"{}\""
                                                  "}";

/// A malformed RenderDocument directive with APL payload without presentationToken.
 static const std::string DOCUMENT_APL_PAYLOAD_MALFORMED = "{"
                                                          "\"token\":\"APL_TOKEN\""
                                                          "}";

/// A malformed RenderDocument directive with APL payload without document.
 static const std::string DOCUMENT_APL_PAYLOAD_MALFORMED_2 = "{"
                                                            "\"presentationToken\":\"APL_TOKEN\""
                                                            "}";

/// A malformed ExecuteCommand directive with APL payload without commands.
 static const std::string EXECUTE_COMMAND_PAYLOAD_MALFORMED = "{"
                                                             "\"presentationToken\":\"APL_TOKEN\""
                                                             "}";

/// A malformed ExecuteCommand directive with APL payload without presentationToken.
 static const std::string EXECUTE_COMMAND_PAYLOAD_MALFORMED_2 = "{"
                                                               "\"token\":\"APL_TOKEN\""
                                                               "}";

// Properly formed execute command
 static const std::string EXECUTE_COMMAND_PAYLOAD = "{"
                                                   "\"presentationToken\":\"APL_TOKEN\","
                                                   "\"commands\":\"{idleCommand}\""
                                                   "}";

 static const std::string TIMEOUT_SETTINGS_CONFIG =
 R"({"alexaPresentationCapabilityAgent":{"displayDocumentInteractionIdleTimeout":500}})";

 // Test window ID
  static const std::string WINDOW_ID = "WINDOW_ID";

/// A visual state request token.
 static const unsigned int STATE_REQUEST_TOKEN = 1;

// clang-format on

/// Mock of AlexaPresentationObserverInterface for testing.
class MockGui : public smartScreenSDKInterfaces::AlexaPresentationObserverInterface {
public:
    MOCK_METHOD2(executeCommands, void(const std::string& jsonPayload, const std::string& token));
    MOCK_METHOD3(
        renderDocument,
        void(const std::string& jsonPayload, const std::string& token, const std::string& windowId));
    MOCK_METHOD0(clearDocument, void());
    MOCK_METHOD0(interruptCommandSequence, void());
};

/// Mock of VisualStateProviderInterface for testing.
class MockVisualStateProvider : public alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface {
public:
    MOCK_METHOD1(provideState, void(const unsigned int stateRequestToken));
};

std::mutex m;
std::condition_variable conditionVariable;

/// Test harness for @c AlexaPresentation class.
class AlexaPresentationTest : public ::testing::Test {
public:
    /// Set up the test harness for running a test.
    void SetUp() override;

    /// Clean up the test harness after running a test.
    void TearDown() override;

    /// A constructor which initializes the promises and futures needed for the test class.
    AlexaPresentationTest() :
            m_wakeAcquireChannelPromise{},
            m_wakeAcquireChannelFuture{m_wakeAcquireChannelPromise.get_future()},
            m_wakeReleaseChannelPromise{},
            m_wakeReleaseChannelFuture{m_wakeReleaseChannelPromise.get_future()} {
    }

protected:
    /// Promise to synchronize acquireChannel calls.
    std::promise<void> m_wakeAcquireChannelPromise;

    /// Future to synchronize acquireChannel calls.
    std::future<void> m_wakeAcquireChannelFuture;

    /// Promise to synchronize releaseChannel calls.
    std::promise<void> m_wakeReleaseChannelPromise;

    /// Future to synchronize releaseChannel calls.
    std::future<void> m_wakeReleaseChannelFuture;

    /// This is the condition variable to be used to control getting of a context in test cases.
    std::condition_variable m_contextTrigger;

    /// mutex for the conditional variables.
    std::mutex m_mutex;

    /// A strict mock that allows the test to fetch context.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockContextManager>> m_mockContextManager;

    /// A strict mock that allows the test to strictly monitor the exceptions being sent.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;

    /// A strict mock that allows the test to strictly monitor the handling of directives.
    std::unique_ptr<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;

    /// @c FocusManager to request focus to the Visual channel.
    std::shared_ptr<smartScreenSDKInterfaces::test::MockFocusManager> m_mockFocusManager;

    /// A strict mock to allow testing of the observer callback.
    std::shared_ptr<StrictMock<MockGui>> m_mockGui;

    /// A pointer to an instance of the AlexaPresentation that will be instantiated per test.
    std::shared_ptr<AlexaPresentation> m_AlexaPresentation;

    /// The mock @c MessageSenderInterface.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockMessageSender>> m_mockMessageSender;

    /// A strict mock to allow testing for visual state provider.
    std::shared_ptr<StrictMock<MockVisualStateProvider>> m_mockVisualStateProvider;

    // The pointer into the @c Executor used by the tested object.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;
};

/*
 * Utility function to generate @c ConfigurationNode from JSON string.
 */
static void setConfig() {
    auto stream = std::shared_ptr<std::stringstream>(new std::stringstream(TIMEOUT_SETTINGS_CONFIG));
    JSONStream jsonStream({stream});
    ConfigurationNode::initialize(jsonStream);
}

void AlexaPresentationTest::SetUp() {
    setConfig();
    m_mockContextManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockContextManager>>();
    m_mockExceptionSender =
        std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockExceptionEncounteredSender>>();
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();
    m_mockFocusManager = std::make_shared<NiceMock<smartScreenSDKInterfaces::test::MockFocusManager>>();
    m_mockGui = std::make_shared<StrictMock<MockGui>>();
    m_mockMessageSender = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockMessageSender>>();
    m_mockVisualStateProvider = std::make_shared<StrictMock<MockVisualStateProvider>>();

    EXPECT_CALL(*m_mockContextManager, setStateProvider(_, _)).Times(Exactly(1));

    m_AlexaPresentation = AlexaPresentation::create(
        m_mockFocusManager,
        m_mockExceptionSender,
        m_mockMessageSender,
        m_mockContextManager,
        m_mockVisualStateProvider);

    m_executor = std::make_shared<alexaClientSDK::avsCommon::utils::threading::Executor>();
    m_AlexaPresentation->setExecutor(m_executor);
    m_AlexaPresentation->addObserver(m_mockGui);

    ON_CALL(*m_mockFocusManager, acquireChannel(_, _, _)).WillByDefault(InvokeWithoutArgs([this] {
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::FOREGROUND);
        return true;
    }));

    ON_CALL(*m_mockFocusManager, releaseChannel(_, _)).WillByDefault(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::NONE);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));
}

void AlexaPresentationTest::TearDown() {
    if (m_AlexaPresentation) {
        m_AlexaPresentation->shutdown();
        m_AlexaPresentation.reset();
    }
}

/**
 * Tests creating the AlexaPresentation with a null contextManager.
 */
TEST_F(AlexaPresentationTest, testNullContextManagerInterface) {
    auto alexaPresentation =
        AlexaPresentation::create(m_mockFocusManager, m_mockExceptionSender, m_mockMessageSender, nullptr);
    ASSERT_EQ(alexaPresentation, nullptr);
}

/**
 * Tests creating the AlexaPresentation with a null focusManagerInterface.
 */
TEST_F(AlexaPresentationTest, testNullFocusManagerInterface) {
    auto alexaPresentation =
        AlexaPresentation::create(nullptr, m_mockExceptionSender, m_mockMessageSender, m_mockContextManager);
    ASSERT_EQ(alexaPresentation, nullptr);
}

/**
 * Tests creating the AlexaPresentation with a null exceptionSender.
 */
TEST_F(AlexaPresentationTest, testNullExceptionSender) {
    auto alexaPresentation =
        AlexaPresentation::create(m_mockFocusManager, nullptr, m_mockMessageSender, m_mockContextManager);
    ASSERT_EQ(alexaPresentation, nullptr);
}

TEST_F(AlexaPresentationTest, testNullMessageSender) {
    auto alexaPresentation =
        AlexaPresentation::create(m_mockFocusManager, m_mockExceptionSender, nullptr, m_mockContextManager);
    ASSERT_EQ(alexaPresentation, nullptr);
}

/**
 * Tests unknown Directive. Expect that the sendExceptionEncountered and setFailed will be called.
 */
TEST_F(AlexaPresentationTest, testUnknownDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(NAMESPACE1, UNKNOWN_DIRECTIVE, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive = AVSDirective::create("", avsMessageHeader, "", attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();
}

/**
 * Verify the request sent to AVS is as expected.
 */
static void verifySendMessage(
    std::shared_ptr<avsCommon::avs::MessageRequest> request,
    const std::string expectedEventName,
    const std::string expectedPayload,
    const std::string expectedNameSpace) {
    rapidjson::Document document;
    document.Parse(request->getJsonContent());
    auto event = document.FindMember(MESSAGE_EVENT_KEY);
    EXPECT_NE(event, document.MemberEnd());

    auto header = event->value.FindMember(MESSAGE_HEADER_KEY);
    EXPECT_NE(header, event->value.MemberEnd());
    auto payload = event->value.FindMember(MESSAGE_PAYLOAD_KEY);
    EXPECT_NE(payload, event->value.MemberEnd());
    EXPECT_EQ(header->value.FindMember(MESSAGE_NAMESPACE_KEY)->value.GetString(), expectedNameSpace);
    EXPECT_EQ(header->value.FindMember(MESSAGE_NAME_KEY)->value.GetString(), expectedEventName);
    EXPECT_NE(header->value.FindMember(MESSAGE_ID)->value.GetString(), "");

    std::string messagePayload;
    avsCommon::utils::json::jsonUtils::convertToValue(payload->value, &messagePayload);
    EXPECT_EQ(messagePayload, expectedPayload);
    EXPECT_EQ(request->attachmentReadersCount(), 0);

    conditionVariable.notify_all();
}

/**
 * Tests when a malformed RenderDocument Directive (without presentationToken) is received.  Expect that the
 * sendExceptionEncountered and setFailed will be called.
 */
TEST_F(AlexaPresentationTest, testMalformedRenderDocumentDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD_MALFORMED, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests when a malformed RenderDocument Directive (without document) is received.  Expect that the
 * sendExceptionEncountered and setFailed will be called.
 */
TEST_F(AlexaPresentationTest, testMalformedRenderDocumentDirective2) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD_MALFORMED_2, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests when a malformed ExecuteCommands Directive is received (without presentationToken).  Expect that the
 * sendExceptionEncountered and setFailed will be called.
 */
TEST_F(AlexaPresentationTest, testMalformedExecuteCommandDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(COMMAND.nameSpace, COMMAND.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, EXECUTE_COMMAND_PAYLOAD_MALFORMED, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests when a malformed ExecuteCommands Directive (without commands) is received.  Expect that the
 * sendExceptionEncountered and setFailed will be called.
 */
TEST_F(AlexaPresentationTest, testMalformedExecuteCommandDirective2) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(COMMAND.nameSpace, COMMAND.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, EXECUTE_COMMAND_PAYLOAD_MALFORMED_2, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests when a ExecuteCommands Directive is received after an APL card id displayed. In this case the
 * ExecuteCommand should fail as presentationToken(APL rendered) != presentationToken(ExecuteCommand).
 */
TEST_F(AlexaPresentationTest, testExecuteCommandAfterMismatchedAPLCard) {
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD_2, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD_2, "APL_TOKEN_2", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN_2", true, "");
    m_executor->waitForSubmittedTasks();

    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    // Create Directive.
    auto attachmentManager1 = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader1 = std::make_shared<AVSMessageHeader>(COMMAND.nameSpace, COMMAND.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive1 =
        AVSDirective::create("", avsMessageHeader1, EXECUTE_COMMAND_PAYLOAD, attachmentManager1, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive1, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_AlexaPresentation->processExecuteCommandsResult(MESSAGE_ID, true, "");
    m_executor->waitForSubmittedTasks();
}

/**
 * Tests when a ExecuteCommands Directive is received after displaying an APL card with matching presentationToken.
 * The command should be successful.
 *
 * @note DISABLED for now. Following up JIRA https://issues.labcollab.net/browse/ARC-871
 */
TEST_F(AlexaPresentationTest, DISABLED_testExecuteCommandAfterRightAPL) {
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));

    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();
    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    // Create Directive.
    auto attachmentManager1 = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader1 = std::make_shared<AVSMessageHeader>(COMMAND.nameSpace, COMMAND.name, MESSAGE_ID_2);
    std::shared_ptr<AVSDirective> directive1 =
        AVSDirective::create("", avsMessageHeader1, EXECUTE_COMMAND_PAYLOAD, attachmentManager1, "");
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    EXPECT_CALL(*m_mockGui, executeCommands(EXECUTE_COMMAND_PAYLOAD, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive1, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID_2);
    m_executor->waitForSubmittedTasks();
}

/**
 * Tests that if clearCard() is called and the card being cleared is APL, then
 * 1. clearDocument() will not be called.
 * 2. DocumentDismissed event will be sent to AVS
 * 3. When provideState is called, we do not call the visual context provider for context since the document has been
 *      clear
 */
TEST_F(AlexaPresentationTest, testAPLClearCard) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));

    EXPECT_CALL(*m_mockGui, clearDocument()).Times(1);

    // Expect a call to getContext as part of sending APL_DISMISSED event.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::NONE);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));

    auto verifyEvent = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, DOCUMENT_DISMISSED_EVENT, EXPECTED_DOCUMENT_DISMISSED_PAYLOAD, NAMESPACE1);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    EXPECT_CALL(*m_mockContextManager, setState(_, _, _, _)).Times(Exactly((1)));

    // Make sure that this is not called, because the last APL has been cleared
    EXPECT_CALL(*m_mockVisualStateProvider, provideState(_)).Times(Exactly(0));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();
    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->clearCard();
    m_AlexaPresentation->provideState(DOCUMENT, STATE_REQUEST_TOKEN);
    m_executor->waitForSubmittedTasks();

    // wait for first call of getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_executor->waitForSubmittedTasks();
    m_AlexaPresentation->onContextAvailable("");

    std::unique_lock<std::mutex> lk(m);
    conditionVariable.wait(lk);
}

/**
 * Tests that when APL is dismissed for timeout, we send Dismissed.
 * When this timeout is followed by another card, we don't send Dismissed again
 * We do not ask for visual context either
 */
TEST_F(AlexaPresentationTest, testAPLTimeout) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(Exactly(1));
    EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::NONE);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));

    // Expect a call to getContext.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));

    auto verifyEvent = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, DOCUMENT_DISMISSED_EVENT, EXPECTED_DOCUMENT_DISMISSED_PAYLOAD, NAMESPACE1);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->onDialogUXStateChanged(
        avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE);
    m_executor->waitForSubmittedTasks();
    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_AlexaPresentation->onContextAvailable("");

    std::unique_lock<std::mutex> lk(m);
    conditionVariable.wait(lk);

    auto attachmentManager2 = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader2 = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID_2);
    std::shared_ptr<AVSDirective> directive2 =
        AVSDirective::create("", avsMessageHeader2, DOCUMENT_APL_PAYLOAD_2, attachmentManager2, "");

    // Re-initializing the uniqe pointer after moved above.
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD_2, "APL_TOKEN_2", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(Exactly(1));
    EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::NONE);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));
    EXPECT_CALL(*m_mockContextManager, setState(_, _, _, _)).Times(Exactly((1)));

    // Make sure that this is not called, because the last APL has been cleared
    EXPECT_CALL(*m_mockVisualStateProvider, provideState(_)).Times(Exactly(0));

    m_AlexaPresentation->provideState(DOCUMENT, STATE_REQUEST_TOKEN);
    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive2, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID_2);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN_2", true, "");
    m_executor->waitForSubmittedTasks();

    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
}

/**
 * Tests that APL is not dismissed while there are active interaction reported.
 */
TEST_F(AlexaPresentationTest, testAPLIdleRespectsGUIActive) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(0);

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processActivityEvent("TEST", smartScreenSDKInterfaces::ActivityEvent::ACTIVATED);
    m_AlexaPresentation->onDialogUXStateChanged(
        avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE);
    m_executor->waitForSubmittedTasks();
}

/**
 * Tests that APL is dismissed as usual when there are no interaction reported.
 */
TEST_F(AlexaPresentationTest, testAPLIdleRespectsGUIInactive) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(1);
    EXPECT_CALL(*m_mockFocusManager, releaseChannel(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_AlexaPresentation->onFocusChanged(avsCommon::avs::FocusState::NONE);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));

    // Expect a call to getContext.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));

    auto verifyEvent = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, DOCUMENT_DISMISSED_EVENT, EXPECTED_DOCUMENT_DISMISSED_PAYLOAD, NAMESPACE1);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processActivityEvent("TEST", smartScreenSDKInterfaces::ActivityEvent::ACTIVATED);
    m_AlexaPresentation->processActivityEvent("TEST", smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
    m_executor->waitForSubmittedTasks();

    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_AlexaPresentation->onContextAvailable("");

    std::unique_lock<std::mutex> lk(m);
    conditionVariable.wait(lk);
}

/**
 * Tests that APL is not dismissed on DEACTIVATED event when DialogUX is on.
 */
TEST_F(AlexaPresentationTest, testAPLIdleRespectsDialogUXWhenGUIInactive) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(0);

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->onDialogUXStateChanged(
        avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::SPEAKING);
    m_AlexaPresentation->processActivityEvent("TEST", smartScreenSDKInterfaces::ActivityEvent::DEACTIVATED);
    m_executor->waitForSubmittedTasks();
}

/**
 * Tests that when APL is dismissed by another card, we send DocumentDismissed
 * for the 1st card with the first token and for the 2nd card with the 2nd token
 */
TEST_F(AlexaPresentationTest, testAPLFollowedByAPL) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    avsCommon::utils::logger::getConsoleLogger()->setLevel(avsCommon::utils::logger::Level::DEBUG9);

    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, DOCUMENT_APL_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD, "APL_TOKEN", WINDOW_ID)).Times(1);
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(AtLeast(1));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN", true, "");
    m_executor->waitForSubmittedTasks();

    // Create another directive
    auto attachmentManager2 = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader2 = std::make_shared<AVSMessageHeader>(DOCUMENT.nameSpace, DOCUMENT.name, MESSAGE_ID_2);
    std::shared_ptr<AVSDirective> directive2 =
        AVSDirective::create("", avsMessageHeader2, DOCUMENT_APL_PAYLOAD_2, attachmentManager, "");

    // Re-initializing the uniqe pointer after moved above.
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    EXPECT_CALL(*m_mockGui, renderDocument(DOCUMENT_APL_PAYLOAD_2, "APL_TOKEN_2", WINDOW_ID)).Times(1);

    // Expect a call to getContext.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));

    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(AtLeast(1));

    auto verifyEvent = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, DOCUMENT_DISMISSED_EVENT, EXPECTED_DOCUMENT_DISMISSED_PAYLOAD, NAMESPACE1);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_AlexaPresentation->CapabilityAgent::preHandleDirective(directive2, std::move(m_mockDirectiveHandlerResult));
    m_AlexaPresentation->CapabilityAgent::handleDirective(MESSAGE_ID_2);

    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_AlexaPresentation->onContextAvailable("");

    std::unique_lock<std::mutex> lk(m);
    conditionVariable.wait(lk);

    m_executor->waitForSubmittedTasks();

    m_AlexaPresentation->processRenderDocumentResult("APL_TOKEN_2", true, "");
    m_executor->waitForSubmittedTasks();

    // clearDocument() is going to be called for the 2nd APL card because it's cleared by timeout.
    EXPECT_CALL(*m_mockGui, clearDocument()).Times(Exactly(1));
    // Expect a call to getContext.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));
    auto verifyEvent2 = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, DOCUMENT_DISMISSED_EVENT, EXPECTED_DOCUMENT_DISMISSED_PAYLOAD_2, NAMESPACE1);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent2));
    m_AlexaPresentation->onDialogUXStateChanged(
        avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::IDLE);
    m_executor->waitForSubmittedTasks();

    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_AlexaPresentation->onContextAvailable("");

    conditionVariable.wait_for(lk, TIMEOUT);
}

TEST_F(AlexaPresentationTest, testSendUserEvent) {
    std::unique_lock<std::mutex> exitLock(m_mutex);
    auto verifyEvent = [](std::shared_ptr<avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, USER_EVENT_EVENT, SAMPLE_USER_EVENT_PAYLOAD, NAMESPACE2);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));
    // Expect a call to getContext.
    EXPECT_CALL(*m_mockContextManager, getContext(_))
        .WillOnce(Invoke(
            [this](std::shared_ptr<ContextRequesterInterface> contextRequester) { m_contextTrigger.notify_one(); }));

    m_AlexaPresentation->sendUserEvent(SAMPLE_USER_EVENT_PAYLOAD);
    // wait for getContext
    m_contextTrigger.wait_for(exitLock, TIMEOUT);
    m_AlexaPresentation->onContextAvailable("");

    std::unique_lock<std::mutex> lk(m);
    conditionVariable.wait(lk);
}

}  // namespace test
}  // namespace alexaPresentation
}  // namespace smartScreenCapabilityAgents
}  // namespace alexaSmartScreenSDK
