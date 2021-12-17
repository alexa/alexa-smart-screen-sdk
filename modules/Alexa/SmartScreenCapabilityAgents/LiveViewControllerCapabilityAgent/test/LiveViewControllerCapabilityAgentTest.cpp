/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AVSCommon/AVS/AVSDirective.h>
#include <AVSCommon/AVS/AVSMessageHeader.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Memory/Memory.h>

#include <MockContextManager.h>
#include <MockExceptionEncounteredSender.h>
#include <MockMessageSender.h>
#include <MockAttachmentManager.h>
#include <MockFocusManager.h>
#include <MockDirectiveHandlerResult.h>
#include <AVSCommon/AVS/FocusState.h>

#include "LiveViewControllerCapabilityAgent/LiveViewControllerCapabilityAgent.h"
#include "RTCSCNativeInterface/RtcscAppClientInterface.h"
#include "SmartScreenSDKInterfaces/LiveViewControllerCapabilityAgentObserverInterface.h"

namespace liveViewController {
namespace test {

using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::utils::memory;
using namespace alexaSmartScreenSDK;
using namespace alexaSmartScreenSDK::smartScreenCapabilityAgents::liveViewController;
using namespace ::testing;

/// AVS Directive payload to use for testing purposes.
// clang-format off
static const std::string TEST_START_DIRECTIVE_PAYLOAD = R"(
{
    "sessionId":"testSessionId",
    "target":{
        "type":"ALEXA_ENDPOINT",
        "endpointId":"testEndpointId"
    },
    "role":"VIEWER",
    "participants":{
        "viewers":[
            {
                "name":"viewerName",
                "hasCameraControl":"true",
                "state":"CONNECTED"
            }
        ],
        "camera":{
            "name":"cameraName",
            "make":"cameraMake",
            "model":"cameraModel",
            "capabilities":"[PHYSICAL_PAN, PHYSICAL_TILT, PHYSICAL_ZOOM]"
        }
    },
    "viewerExperience":{
        "suggestedDisplay":{
            "displayMode":"FULL_SCREEN",
            "overlayType":"NONE",
            "overlayPosition":"TOP_RIGHT"
        },
        "audioProperties":{
            "talkMode":"PRESS_AND_HOLD",
            "concurrentTwoWayTalk":"ENABLED",
            "microphoneState":"UNMUTED",
            "speakerState":"UNMUTED"
        },
        "liveViewTrigger":"USER_ACTION",
        "idleTimeoutInMilliseconds":1000
    }
}
)";

static const std::string TEST_STOP_DIRECTIVE_PAYLOAD = R"(
{
    "sessionId":"testSessionId",
    "target":{
        "type":"ALEXA_ENDPOINT",
        "endpointId":"testEndpointId"
    }
}
)";

// clang-format on

/// Expected LiveViewStarted payload for test purposes
static const std::string EXPECTED_LIVE_VIEW_STARTED_PAYLOAD =
    R"({"sessionId":"testSessionId","target":{"endpointId":"testEndpointId","type":"ALEXA_ENDPOINT"}})";

/// Expected LiveViewStopped payload for test purposes
static const std::string EXPECTED_LIVE_VIEW_STOPPED_PAYLOAD =
    R"({"sessionId":"testSessionId","target":{"endpointId":"testEndpointId","type":"ALEXA_ENDPOINT"}})";

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

/// The namespace registered for this capability agent.
static const std::string NAMESPACE{"Alexa.Camera.LiveViewController"};

/// The StartLiveView directive signature.
static const NamespaceAndName STARTLIVEVIEW{NAMESPACE, "StartLiveView"};

/// The StartLiveView directive signature.
static const NamespaceAndName STOPLIVEVIEW{NAMESPACE, "StopLiveView"};

/// The LiveViewStarted event signature.
static const std::string LIVE_VIEW_STARTED_EVENT{"LiveViewStarted"};

/// The LiveViewStopped event signature.
static const std::string LIVE_VIEW_STOPPED_EVENT{"LiveViewStopped"};

/// The @c MessageId identifer.
static const std::string MESSAGE_ID("messageId");

/// The @c MessageId identifer 2.
static const std::string MESSAGE_ID_2("messageId_2");

/// Test session id identifier.
static const std::string TEST_SESSION_ID("testSessionId");

/// The test manufacturer name
static const std::string MANUFACTURER_NAME("testManufacturerName");

/// The test camera friendly name
static const std::string CAMERA_FRIENDLY_NAME("testCameraFriendlyName");

/// The interface type
static const std::string INTERFACE_TYPE("AlexaInterface");

/// The interface name
static const std::string INTERFACE_NAME("Alexa.Camera.LiveViewController");

/// The interface version
static const std::string INTERFACE_VERSION("1.7");

class MockObserver
        : public alexaSmartScreenSDK::smartScreenSDKInterfaces::LiveViewControllerCapabilityAgentObserverInterface {
public:
    MOCK_METHOD3(
        renderCamera,
        void(
            const std::string& payload,
            smartScreenSDKInterfaces::AudioState microphoneAudioState,
            smartScreenSDKInterfaces::ConcurrentTwoWayTalk concurrentTwoWayTalk));
    MOCK_METHOD1(onCameraStateChanged, void(smartScreenSDKInterfaces::CameraState cameraState));
    MOCK_METHOD0(onFirstFrameRendered, void());
    MOCK_METHOD0(clearCamera, void());
};

class MockRtcscAppClient : public rtc::native::RtcscAppClientInterface {
public:
    MOCK_METHOD2(
        registerAppClientListener,
        rtc::native::RtcscErrorCode(
            const rtc::native::AppInfo& appInfo,
            rtc::native::RtcscAppClientListenerInterface* appClientListener));
    MOCK_METHOD1(unregisterAppClientListener, rtc::native::RtcscErrorCode(const rtc::native::AppInfo& appInfo));
    MOCK_METHOD2(
        registerMetricsPublisherListener,
        rtc::native::RtcscErrorCode(
            const rtc::native::AppInfo& appInfo,
            rtc::native::RtcscMetricsPublisherListenerInterface* metricsPublisherListener));
    MOCK_METHOD1(unregisterMetricsPublisherListener, rtc::native::RtcscErrorCode(const rtc::native::AppInfo& appInfo));
    MOCK_METHOD2(setLocalAudioState, rtc::native::RtcscErrorCode(const std::string& sessionId, bool audioEnabled));
    MOCK_METHOD2(setLocalVideoState, rtc::native::RtcscErrorCode(const std::string& sessionId, bool videoEnabled));
    MOCK_METHOD2(setRemoteAudioState, rtc::native::RtcscErrorCode(const std::string& sessionId, bool audioEnabled));
    MOCK_METHOD1(acceptSession, rtc::native::RtcscErrorCode(const std::string& sessionId));
    MOCK_METHOD2(
        disconnectSession,
        rtc::native::RtcscErrorCode(
            const std::string& sessionId,
            rtc::native::RtcscAppDisconnectCode rtcscAppDisconnectCode));
    MOCK_METHOD2(
        switchCamera,
        rtc::native::RtcscErrorCode(const std::string& sessionId, const std::string& cameraName));
    MOCK_METHOD1(signalReadyForSession, rtc::native::RtcscErrorCode(const std::string& sessionId));
    MOCK_METHOD3(
        setVideoEffect,
        rtc::native::RtcscErrorCode(
            const std::string& sessionId,
            const rtc::native::VideoEffect& videoEffect,
            int videoEffectDurationMs));
    MOCK_METHOD2(
        registerDataChannelListener,
        bool(const std::string& sessionId, rtc::native::RtcscDataChannelListenerInterface* dataChannelListener));
    MOCK_METHOD1(unregisterDataChannelListener, bool(const std::string& sessionId));
    MOCK_METHOD4(
        sendData,
        bool(const std::string& sessionId, const std::string& label, const std::string& data, bool binary));
    MOCK_METHOD3(
        registerSurfaceConsumer,
        void(
            const std::string& sessionId,
            std::shared_ptr<rtc::native::RtcscSurfaceConsumerInterface> surfaceConsumer,
            rtc::native::MediaSide side));
    MOCK_METHOD2(unregisterSurfaceConsumer, void(const std::string& sessionId, rtc::native::MediaSide side));
};

class LiveViewControllerCapabilityAgentTest : public ::testing::Test {
public:
    // Set up the test harness for running a test.
    void SetUp() override;

    // Clean up the test harness after running a test.
    void TearDown() override;

protected:
    // A strict mock that allows the test to fetch context.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockContextManager>> m_mockContextManager;

    // A strict mock that allows the test to strictly monitor the exceptions being sent.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockExceptionEncounteredSender>> m_mockExceptionSender;

    // The mock @c MessageSenderInterface.
    std::shared_ptr<StrictMock<smartScreenSDKInterfaces::test::MockMessageSender>> m_mockMessageSender;

    /// A strict mock that allows the test to strictly monitor the handling of directives.
    std::unique_ptr<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>
        m_mockDirectiveHandlerResult;

    // The mock @c FocusManagerInterface
    std::shared_ptr<NiceMock<smartScreenSDKInterfaces::test::MockFocusManager>> m_mockFocusManager;

    // The pointer into the @c Executor used by the tested object.
    std::shared_ptr<alexaClientSDK::avsCommon::utils::threading::Executor> m_executor;

    // The mock @c RtcscAppClientInterface.
    std::shared_ptr<StrictMock<MockRtcscAppClient>> m_mockRtcscAppClient;

    // A pointer to an instance of the LiveViewControllerCapabilityAgent that will be instantiated per test.
    std::shared_ptr<LiveViewControllerCapabilityAgent> m_liveViewControllerCapabilityAgent;

    // A strict mock to allow testing of the observer callbacks.
    std::shared_ptr<StrictMock<MockObserver>> m_mockObserver;
};

void LiveViewControllerCapabilityAgentTest::SetUp() {
    m_mockContextManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockContextManager>>();
    m_mockExceptionSender =
        std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockExceptionEncounteredSender>>();
    m_mockMessageSender = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockMessageSender>>();
    m_mockFocusManager = std::make_shared<NiceMock<smartScreenSDKInterfaces::test::MockFocusManager>>();
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    m_liveViewControllerCapabilityAgent = LiveViewControllerCapabilityAgent::create(
        m_mockFocusManager, m_mockMessageSender, m_mockContextManager, m_mockExceptionSender);

    m_executor = std::make_shared<alexaClientSDK::avsCommon::utils::threading::Executor>();
    m_liveViewControllerCapabilityAgent->setExecutor(m_executor);

    m_mockRtcscAppClient = std::make_shared<StrictMock<MockRtcscAppClient>>();
    m_liveViewControllerCapabilityAgent->setRtcscAppClient(m_mockRtcscAppClient);

    m_mockObserver = std::make_shared<StrictMock<MockObserver>>();

    m_liveViewControllerCapabilityAgent->addObserver(m_mockObserver);

    ON_CALL(*m_mockFocusManager, acquireChannel(_, _, _)).WillByDefault(InvokeWithoutArgs([this] {
        m_liveViewControllerCapabilityAgent->onFocusChanged(
            alexaClientSDK::avsCommon::avs::FocusState::FOREGROUND,
            alexaClientSDK::avsCommon::avs::MixingBehavior::UNDEFINED);
        return true;
    }));

    ON_CALL(*m_mockFocusManager, releaseChannel(_, _)).WillByDefault(InvokeWithoutArgs([this] {
        auto releaseChannelSuccess = std::make_shared<std::promise<bool>>();
        std::future<bool> returnValue = releaseChannelSuccess->get_future();
        m_liveViewControllerCapabilityAgent->onFocusChanged(
            alexaClientSDK::avsCommon::avs::FocusState::NONE,
            alexaClientSDK::avsCommon::avs::MixingBehavior::UNDEFINED);
        releaseChannelSuccess->set_value(true);
        return returnValue;
    }));
}

void LiveViewControllerCapabilityAgentTest::TearDown() {
    if (m_liveViewControllerCapabilityAgent) {
        m_liveViewControllerCapabilityAgent->removeObserver(m_mockObserver);
        m_liveViewControllerCapabilityAgent->shutdown();
        m_liveViewControllerCapabilityAgent.reset();
    }
}

/**
 * Verify the request sent to AVS is as expected.
 */
static void verifySendMessage(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request,
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
    alexaClientSDK::avsCommon::utils::json::jsonUtils::convertToValue(payload->value, &messagePayload);
    EXPECT_EQ(messagePayload, expectedPayload);
    EXPECT_EQ(request->attachmentReadersCount(), 0);
}

/**
 * Tests creating the LiveViewControllerCapabilityAgent with a null focusManager.
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testNullFocusManagerInterface) {
    auto liveViewControllerCapabilityAgent = LiveViewControllerCapabilityAgent::create(
        nullptr, m_mockMessageSender, m_mockContextManager, m_mockExceptionSender);
    ASSERT_EQ(liveViewControllerCapabilityAgent, nullptr);
}

/**
 * Tests creating the LiveViewControllerCapabilityAgent with a null messageSender.
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testNullMessageSenderInterface) {
    auto liveViewControllerCapabilityAgent = LiveViewControllerCapabilityAgent::create(
        m_mockFocusManager, nullptr, m_mockContextManager, m_mockExceptionSender);
    ASSERT_EQ(liveViewControllerCapabilityAgent, nullptr);
}

/**
 * Tests creating the LiveViewControllerCapabilityAgent with a null contextManager.
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testNullContextManagerInterface) {
    auto liveViewControllerCapabilityAgent = LiveViewControllerCapabilityAgent::create(
        m_mockFocusManager, m_mockMessageSender, nullptr, m_mockExceptionSender);
    ASSERT_EQ(liveViewControllerCapabilityAgent, nullptr);
}

/**
 * Tests creating the LiveViewControllerCapabilityAgent with a null exceptionSender.
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testNullExceptionSenderInterface) {
    auto liveViewControllerCapabilityAgent = LiveViewControllerCapabilityAgent::create(
        m_mockFocusManager, m_mockMessageSender, m_mockContextManager, nullptr);
    ASSERT_EQ(liveViewControllerCapabilityAgent, nullptr);
}

/**
 * Tests SuccessfulHandleDirective
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testSuccessfulHandleDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    ON_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _))
        .WillByDefault(Return(rtc::native::RtcscErrorCode::SUCCESS));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, onCameraStateChanged(_)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, renderCamera(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    auto verifyEvent = [](std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, LIVE_VIEW_STARTED_EVENT, EXPECTED_LIVE_VIEW_STARTED_PAYLOAD, INTERFACE_NAME);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);
    m_executor->waitForSubmittedTasks();
}

/**
 * Tests handleDirective with unknownDirective
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testHandleUnknownDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>("unknownNameSpace", "unknownName", MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests FailedHandleDirective
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testFailedHandleDirective) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, R"({"name":"hello"})", attachmentManager, "");

    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(1));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests cancelDirective
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testCancelDirective) {
    EXPECT_CALL(*m_mockExceptionSender, sendExceptionEncountered(_, _, _)).Times(Exactly(0));
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setFailed(_)).Times(Exactly(0));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::cancelDirective(MESSAGE_ID);

    m_executor->waitForSubmittedTasks();
}

/**
 * Tests a full sequence of launching a camera feed, adjusting audio states, and clearing it.
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testRenderCameraStreamingScreen) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, onCameraStateChanged(_)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, renderCamera(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    auto verifyEvent = [](std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, LIVE_VIEW_STARTED_EVENT, EXPECTED_LIVE_VIEW_STARTED_PAYLOAD, INTERFACE_NAME);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();
    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);

    m_executor->waitForSubmittedTasks();

    auto attachmentManager2 = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader2 =
        std::make_shared<AVSMessageHeader>(STOPLIVEVIEW.nameSpace, STOPLIVEVIEW.name, MESSAGE_ID_2);
    std::shared_ptr<AVSDirective> directive2 =
        AVSDirective::create("", avsMessageHeader2, TEST_STOP_DIRECTIVE_PAYLOAD, attachmentManager2, "");

    // Re-initializing the uniqe pointer after moved above.
    m_mockDirectiveHandlerResult =
        make_unique<StrictMock<smartScreenSDKInterfaces::test::MockDirectiveHandlerResult>>();

    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, clearCamera()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, disconnectSession(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    auto verifyEvent2 = [](std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, LIVE_VIEW_STOPPED_EVENT, EXPECTED_LIVE_VIEW_STOPPED_PAYLOAD, INTERFACE_NAME);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent2));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive2, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID_2);
    m_executor->waitForSubmittedTasks();
    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);

    m_executor->waitForSubmittedTasks();
}

/**
 * Test setLocalAudioState
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testSetLocalAudioState) {
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, onCameraStateChanged(_)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, renderCamera(_, _, _)).Times(Exactly(1));

    auto verifyEvent = [](std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, LIVE_VIEW_STARTED_EVENT, EXPECTED_LIVE_VIEW_STARTED_PAYLOAD, INTERFACE_NAME);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));
    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);

    m_executor->waitForSubmittedTasks();
}

/**
 * Test clearLiveView
 */
TEST_F(LiveViewControllerCapabilityAgentTest, testClearLiveView) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    ON_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _))
        .WillByDefault(Return(rtc::native::RtcscErrorCode::SUCCESS));
    ON_CALL(*m_mockMessageSender, sendMessage(_)).WillByDefault(Return());
    EXPECT_CALL(*m_mockDirectiveHandlerResult, setCompleted()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(2));
    EXPECT_CALL(*m_mockObserver, onCameraStateChanged(_)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, renderCamera(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    m_liveViewControllerCapabilityAgent->CapabilityAgent::preHandleDirective(
        directive, std::move(m_mockDirectiveHandlerResult));
    m_liveViewControllerCapabilityAgent->CapabilityAgent::handleDirective(MESSAGE_ID);
    m_executor->waitForSubmittedTasks();

    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);
    m_executor->waitForSubmittedTasks();

    EXPECT_CALL(*m_mockObserver, clearCamera()).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, disconnectSession(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    m_liveViewControllerCapabilityAgent->clearLiveView();
    m_executor->waitForSubmittedTasks();

    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);
    m_executor->waitForSubmittedTasks();
}

TEST_F(LiveViewControllerCapabilityAgentTest, testGetCapabilityConfigurations) {
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> configurations;
    configurations = m_liveViewControllerCapabilityAgent->getCapabilityConfigurations();

    ASSERT_EQ(configurations.size(), 1);
    ASSERT_EQ(configurations.begin()->get()->type, INTERFACE_TYPE);
    ASSERT_EQ(configurations.begin()->get()->interfaceName, INTERFACE_NAME);
    ASSERT_EQ(configurations.begin()->get()->version, INTERFACE_VERSION);
}

TEST_F(LiveViewControllerCapabilityAgentTest, testHandleDirectiveImmediately) {
    // Create Directive.
    auto attachmentManager = std::make_shared<StrictMock<smartScreenSDKInterfaces::test::MockAttachmentManager>>();
    auto avsMessageHeader = std::make_shared<AVSMessageHeader>(STARTLIVEVIEW.nameSpace, STARTLIVEVIEW.name, MESSAGE_ID);
    std::shared_ptr<AVSDirective> directive =
        AVSDirective::create("", avsMessageHeader, TEST_START_DIRECTIVE_PAYLOAD, attachmentManager, "");

    ON_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _))
        .WillByDefault(Return(rtc::native::RtcscErrorCode::SUCCESS));
    EXPECT_CALL(*m_mockRtcscAppClient, registerAppClientListener(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, onCameraStateChanged(_)).Times(Exactly(1));
    EXPECT_CALL(*m_mockObserver, renderCamera(_, _, _)).Times(Exactly(1));
    EXPECT_CALL(*m_mockRtcscAppClient, setLocalAudioState(_, true))
        .WillOnce(Return(rtc::native::RtcscErrorCode::SUCCESS));

    auto verifyEvent = [](std::shared_ptr<alexaClientSDK::avsCommon::avs::MessageRequest> request) {
        verifySendMessage(request, LIVE_VIEW_STARTED_EVENT, EXPECTED_LIVE_VIEW_STARTED_PAYLOAD, INTERFACE_NAME);
    };
    EXPECT_CALL(*m_mockMessageSender, sendMessage(_)).Times(Exactly(1)).WillOnce(Invoke(verifyEvent));

    m_liveViewControllerCapabilityAgent->handleDirectiveImmediately(directive);
    m_executor->waitForSubmittedTasks();

    m_liveViewControllerCapabilityAgent->setMicrophoneState(true);
    m_executor->waitForSubmittedTasks();
}

TEST_F(LiveViewControllerCapabilityAgentTest, testGetConfiguration) {
    m_liveViewControllerCapabilityAgent->getConfiguration();
}

TEST_F(LiveViewControllerCapabilityAgentTest, testRtcscCallbacks) {
    ON_CALL(*m_mockRtcscAppClient, unregisterAppClientListener(_))
        .WillByDefault(Return(rtc::native::RtcscErrorCode::SUCCESS));
    EXPECT_CALL(*m_mockRtcscAppClient, unregisterAppClientListener(_)).Times(Exactly(1));

    m_liveViewControllerCapabilityAgent->onSessionAvailable(TEST_SESSION_ID);
    m_liveViewControllerCapabilityAgent->onSessionRemoved(TEST_SESSION_ID);
    m_liveViewControllerCapabilityAgent->onError(rtc::native::RtcscErrorCode::SUCCESS, "", TEST_SESSION_ID);
    m_liveViewControllerCapabilityAgent->onSessionStateChanged(TEST_SESSION_ID, rtc::native::SessionState::ACTIVE);
    m_liveViewControllerCapabilityAgent->onMediaStatusChanged(
        TEST_SESSION_ID, rtc::native::MediaSide::LOCAL, rtc::native::MediaType::AUDIO, true);
    m_liveViewControllerCapabilityAgent->onVideoEffectChanged(TEST_SESSION_ID, rtc::native::VideoEffect::NONE, 0);
    m_liveViewControllerCapabilityAgent->onMediaConnectionStateChanged(
        TEST_SESSION_ID, rtc::native::MediaConnectionState::CONNECTING);
    m_liveViewControllerCapabilityAgent->onFirstFrameReceived(TEST_SESSION_ID, rtc::native::MediaType::AUDIO);
    m_liveViewControllerCapabilityAgent->onFirstFrameRendered(TEST_SESSION_ID, rtc::native::MediaSide::REMOTE);

    m_executor->waitForSubmittedTasks();
}

}  // namespace test
}  // namespace liveViewController
