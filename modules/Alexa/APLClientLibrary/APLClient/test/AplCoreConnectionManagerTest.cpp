#include "APLClient/AplCoreConnectionManager.h"
#include "MockAplOptionsInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <APLClient/AplCoreTextMeasurement.h>
#include <APLClient/Telemetry/NullAplMetricsRecorder.h>

namespace APLClient {
namespace test {

using namespace ::testing;

static const std::string DATA = "{}";

static const std::string VIEWPORT =
    "["
    "  {"
    "    \"mode\": \"HUB\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 1024,"
    "    \"maxWidth\": 1024,"
    "    \"minHeight\": 600,"
    "    \"maxHeight\": 600"
    "  },"
    "  {"
    "    \"mode\": \"HUB\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 1280,"
    "    \"maxWidth\": 1280,"
    "    \"minHeight\": 800,"
    "    \"maxHeight\": 800"
    "  },"
    "  {"
    "    \"mode\": \"HUB\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 960,"
    "    \"maxWidth\": 960,"
    "    \"minHeight\": 480,"
    "    \"maxHeight\": 480"
    "  },"
    "  {"
    "    \"mode\": \"HUB\","
    "    \"shape\": \"ROUND\","
    "    \"minWidth\": 480,"
    "    \"maxWidth\": 480,"
    "    \"minHeight\": 480,"
    "    \"maxHeight\": 480"
    "  },"
    "  {"
    "    \"mode\": \"TV\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 960,"
    "    \"maxWidth\": 960,"
    "    \"minHeight\": 540,"
    "    \"maxHeight\": 540"
    "  },"
    "  {"
    "    \"mode\": \"TV\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 960,"
    "    \"maxWidth\": 960,"
    "    \"minHeight\": 200,"
    "    \"maxHeight\": 200"
    "  },"
    "  {"
    "    \"mode\": \"TV\","
    "    \"shape\": \"RECTANGLE\","
    "    \"minWidth\": 300,"
    "    \"maxWidth\": 300,"
    "    \"minHeight\": 540,"
    "    \"maxHeight\": 540"
    "  }"
    "]";

static const std::string DOCUMENT =
    "{"
    "  \"type\": \"APL\","
    "  \"version\": \"1.0\","
    "  \"theme\": \"light\","
    "  \"description\": \"This is a sample APL document\","
    "  \"import\": [],"
    "  \"layouts\": {"
    "    \"Box\": {"
    "      \"item\": {"
    "        \"type\": \"VectorGraphic\","
    "        \"width\": \"50dp\","
    "        \"height\": \"50dp\","
    "        \"style\": \"focusStyle\","
    "        \"source\": \"box\""
    "      }"
    "    }"
    "  },"
    "  \"mainTemplate\": {"
    "    \"parameters\": ["
    "      \"payload\""
    "    ],"
    "    \"item\": {"
    "          \"type\": \"Container\","
    "          \"items\": ["
    "            {"
    "              \"type\": \"Text\","
    "              \"id\": \"COMP1\","
    "              \"width\": \"100%\","
    "              \"text\": \"Hello World\","
    "              \"fontSize\": 50"
    "            },"
    "            {"
    "              \"type\": \"Video\","
    "              \"id\": \"video\","
    "              \"height\": 300,"
    "              \"width\": 716.8,"
    "              \"top\": 10,"
    "              \"left\": 100,"
    "              \"autoplay\": true,"
    "              \"audioTrack\": \"background\","
    "              \"source\": ["
    "                {"
    "                  \"url\": \"URL\""
    "                }"
    "              ]"
    "            },"
    "            {"
    "              \"id\": \"textBox\","
    "              \"type\": \"Text\","
    "              \"text\": \"Hello\","
    "              \"fontSize\": 50"
    "            },"
    "            {"
    "              \"id\" : \"GRAPHIC\","
    "              \"type\": \"Box\","
    "              \"position\": \"absolute\","
    "              \"top\": 0,"
    "              \"left\": 225"
    "            }"
    "            ]"
    "      }"
    "   }"
    "}";

static const std::string BUILD_PAYLOAD =
    "{"
    "  \"type\":\"build\","
    "  \"payload\":"
    "  {"
    "    \"agentName\":\"SmartScreenSDK\","
    "    \"agentVersion\":\"1.0\","
    "    \"allowOpenUrl\":false,"
    "    \"disallowVideo\":false,"
    "    \"animationQuality\":\"normal\","
    "    \"width\":1920,\"height\":1080,"
    "    \"shape\":\"RECTANGLE\","
    "    \"dpi\":160,"
    "    \"mode\":\"TV\""
    "  }"
    "}";

static const std::string EVENT_PAYLOAD =
    "{"
    "  \"type\": \"APL\","
    "  \"version\": \"1.1\","
    "  \"mainTemplate\": {"
    "    \"items\": {"
    "      \"type\": \"Pager\","
    "      \"id\": \"myPager\","
    "      \"width\": 100,"
    "      \"height\": 100,"
    "      \"navigation\": \"normal\","
    "      \"items\": {"
    "        \"type\": \"Text\","
    "        \"id\": \"id${data}\","
    "        \"text\": \"TEXT${data}\","
    "        \"speech\": \"URL${data}\""
    "      },"
    "      \"data\": [ 1, 2, 3, 4, 5 ],"
    "      \"onPageChanged\": {"
    "        \"type\": \"SendEvent\","
    "        \"arguments\": ["
    "          \"${event.target.page}\""
    "        ]"
    "      }"
    "    }"
    "  }"
    "}";

static const std::string DEFAULT_PARAM_BINDING = "payload";

static const std::string APL_COMMAND_EXECUTION{"APLCommandExecution"};

static const std::string SEQNO_KEY = "seqno";

/// Test harness for @c AplCoreConnectionManagerTest class.
class AplCoreConnectionManagerTest : public ::testing::Test {
public:
    /// Set up the test harness for running a test.
    void SetUp() override;

    /// Clean up the test harness after running a test.
    void TearDown() override;

    /// Setting up m_Root
    void BuildDocument(const std::string, const std::string, const std::string);

protected:
    std::shared_ptr<MockAplOptionsInterface> m_mockAplOptions;

    std::shared_ptr<AplCoreConnectionManager> m_aplCoreConnectionManager;

    std::shared_ptr<Telemetry::AplMetricsRecorderInterface> m_metricsRecorder;

    AplConfigurationPtr m_aplConfiguration;
};

void AplCoreConnectionManagerTest::SetUp() {
    m_mockAplOptions = std::make_shared<NiceMock<MockAplOptionsInterface>>();
    m_aplConfiguration = std::make_shared<AplConfiguration>(m_mockAplOptions);
    m_aplCoreConnectionManager = std::make_shared<AplCoreConnectionManager>(m_aplConfiguration);
}

void AplCoreConnectionManagerTest::TearDown() {
    if (m_aplCoreConnectionManager) {
        m_aplCoreConnectionManager.reset();
    }
}

void AplCoreConnectionManagerTest::BuildDocument(
    const std::string document,
    const std::string data,
    const std::string viewport) {
    auto content = apl::Content::create(document);
    m_aplCoreConnectionManager->setSupportedViewports(viewport);
    m_aplCoreConnectionManager->setContent(content, "");
    // this is required in order to set content state to ready
    content->addData(DEFAULT_PARAM_BINDING, data);
    m_aplCoreConnectionManager->handleMessage(BUILD_PAYLOAD);
}

TEST_F(AplCoreConnectionManagerTest, SetContentSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);

    auto content = apl::Content::create(DOCUMENT);

    m_aplCoreConnectionManager->setContent(content, "");
}

TEST_F(AplCoreConnectionManagerTest, SetSupportedViewPortsSuccess) {
    m_aplCoreConnectionManager->setSupportedViewports(VIEWPORT);
}

/**
 * Tests BlockingSend function by setting a promise when sendMessage function
 * is called. If future is set correctly, shouldHandleMessage function should called.
 * shouldHandleMessage function sets m_replyPromise in the blockingSend function
 */
TEST_F(AplCoreConnectionManagerTest, BlockingSendSuccess) {
    std::promise<bool> promise = std::promise<bool>();
    std::future<bool> future = promise.get_future();
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(Exactly(1)).WillOnce(InvokeWithoutArgs([&promise] {
        promise.set_value(true);
    }));

    auto measureMsg = AplCoreViewhostMessage("measure");
    auto* aplCoreConnectionManager = m_aplCoreConnectionManager.get();
    rapidjson::Document result;
    rapidjson::Document* resultAddress = &result;
    auto blockingSend = [&aplCoreConnectionManager, &measureMsg, &resultAddress]() {
        *resultAddress = aplCoreConnectionManager->blockingSend(measureMsg, std::chrono::milliseconds(3000));
    };
    std::thread thread(blockingSend);

    auto status = future.wait_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(status == std::future_status::ready);
    if (status == std::future_status::ready) {
        const std::string msg =
            "{"
            "    \"" +
            SEQNO_KEY +
            "\": 1"
            "}";
        aplCoreConnectionManager->shouldHandleMessage(msg);
    }

    thread.join();

    ASSERT_TRUE(result.IsObject());
}

/**
 * Tests HandleMessage function with build type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleBuildSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, true, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);
}

TEST_F(AplCoreConnectionManagerTest, ExecuteCommandsSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(6);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(11);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(2).WillRepeatedly(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onActivityStarted(_, APL_COMMAND_EXECUTION)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onCommandExecutionComplete(_, true)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, APL_COMMAND_EXECUTION)).Times(2);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "{"
        "  \"commands\": ["
        "    {"
        "      \"type\": \"SetValue\","
        "      \"componentId\": \"textBox\","
        "      \"property\": \"text\","
        "      \"value\": \"Hi\""
        "    }"
        "  ]"
        "}";
    m_aplCoreConnectionManager->executeCommands(payload, "");
    // Calling this to resolve pending events
    m_aplCoreConnectionManager->onUpdateTick();
}

TEST_F(AplCoreConnectionManagerTest, ExecuteCommandsInterrupt) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onActivityStarted(_, APL_COMMAND_EXECUTION)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onCommandExecutionComplete(_, false)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, APL_COMMAND_EXECUTION)).Times(2);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "{"
        "  \"commands\": ["
        "    {"
        "      \"type\": \"SetValue\","
        "      \"componentId\": \"textBox\","
        "      \"property\": \"text\","
        "      \"value\": \"Hi\""
        "    }"
        "  ]"
        "}";
    m_aplCoreConnectionManager->executeCommands(payload, "");
    m_aplCoreConnectionManager->interruptCommandSequence();
}

/**
 * Tests HandleMessage function with update type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleUpdateSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const ::std::string payload =
        "{"
        "  \"type\":\"update\","
        "  \"payload\":"
        "  {"
        "       \"id\":\"COMP1\","
        "       \"type\":1,"
        "       \"value\":1"
        "  }"
        "}";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with updateMedia type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleUpdateMediaSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const ::std::string payload =
        "{"
        "  \"type\":\"updateMedia\","
        "  \"payload\":"
        "  {"
        "    \"id\": \"video\","
        "    \"mediaState\":"
        "    {"
        "      \"currentTime\":0,"
        "      \"duration\":62.625,"
        "      \"ended\":false,"
        "      \"paused\":true,"
        "      \"trackCount\":50,"
        "      \"trackIndex\":0"
        "    },"
        "    \"fromEvent\":false"
        "    }"
        "  }";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with updateGraphic type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleGraphicUpdateSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const ::std::string payload =
        "{"
        "\"type\" : \"updateGraphic\","
        "\"payload\" : "
        "{"
        "\"id\" : \"GRAPHIC\","
        "\"avg\" : \"{ \\\"type\\\":\\\"AVG\\\", \\\"version\\\":"
        " \\\"1.0\\\", \\\"height\\\": 100, \\\"width\\\": 100}\""
        "}"
        "}";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with response type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleEventResponseSuccess) {

     EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
     EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(2);
     EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(8);
     EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(2).WillRepeatedly(Return(std::chrono::milliseconds()));
     EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
     EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);
     EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, APL_COMMAND_EXECUTION)).Times(2);
     EXPECT_CALL(*m_mockAplOptions, onActivityStarted(_, APL_COMMAND_EXECUTION)).Times(1);

     BuildDocument(EVENT_PAYLOAD, DATA, VIEWPORT);

     //adding an even to m_Root
     const std::string commands="{"
                           "  \"commands\": ["
                           "    {"
                           "      \"type\": \"SetPage\","
                           "      \"componentId\": \"myPager\","
                           "      \"position\": \"relative\","
                           "      \"value\": \"2\""
                           "    }"
                           "  ]"
                           "}";
     m_aplCoreConnectionManager->executeCommands(commands, "");
     //popping the event from m_Root and adding to m_PendingEvents
     m_aplCoreConnectionManager->onUpdateTick();

     const std::string payload=
                    "  {"
                    "    \"type\":\"response\","
                    "    \"payload\":{"
                    "      \"event\":7"
                    "    }"
                    "  }";
     m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with ensureLayout type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleEnsureLayoutSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(10);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "  {"
        "    \"type\":\"ensureLayout\","
        "    \"payload\":"
        "    {"
        "      \"id\":\"COMP1\""
        "    }"
        "  } ";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with scrollToRectInComponent type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleScrollToRectInComponentSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "  {"
        "    \"type\":\"scrollToRectInComponent\","
        "    \"payload\":"
        "    {"
        "      \"id\":\"COMP1\","
        "      \"x\" : 0,"
        "      \"y\" : 200,"
        "      \"width\" : 1000,"
        "      \"height\" : 500,"
        "      \"align\" : 1"
        "    }"
        "  } ";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with handleKeyboard type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleKeyboardSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(10);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "{"
        "    \"type\":\"handleKeyboard\","
        "    \"payload\":"
        "    {"
        "      \"messageId\":\"msg\","
        "      \"keyType\":0,"
        "      \"code\":\"ArrowDown\","
        "      \"key\":\"ArrowDown\","
        "      \"repeat\":false,"
        "      \"altKey\":false,"
        "      \"ctrlKey\":false,"
        "      \"metaKey\":false,"
        "      \"shiftKey\":false"
        "    }"
        "}";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with updateCursorPosition type.
 */
TEST_F(AplCoreConnectionManagerTest, HandleUpdateCursorPositionSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "{"
        "  \"type\":\"updateCursorPosition\","
        "  \"payload\":{"
        "    \"x\":1934,"
        "    \"y\":300"
        "    }"
        "}";
    m_aplCoreConnectionManager->handleMessage(payload);
}

/**
 * Tests HandleMessage function with handlePointerEvent type.
 */
TEST_F(AplCoreConnectionManagerTest, HandlePointerEventSuccess) {
    EXPECT_CALL(*m_mockAplOptions, resetViewhost(_)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onRenderingEvent(_, _)).Times(5);
    EXPECT_CALL(*m_mockAplOptions, sendMessage(_, _)).Times(9);
    EXPECT_CALL(*m_mockAplOptions, getTimezoneOffset()).Times(1).WillOnce(Return(std::chrono::milliseconds()));
    EXPECT_CALL(*m_mockAplOptions, onActivityEnded(_, _)).Times(1);
    EXPECT_CALL(*m_mockAplOptions, onSetDocumentIdleTimeout(_, _)).Times((1));
    EXPECT_CALL(*m_mockAplOptions, onRenderDocumentComplete(_, _, _)).Times(1);

    BuildDocument(DOCUMENT, DATA, VIEWPORT);

    const std::string payload =
        "{"
        "  \"type\":\"handlePointerEvent\","
        "  \"payload\":"
        "  {"
        "    \"pointerEventType\":3,"
        "    \"x\":800,"
        "    \"y\":394,"
        "    \"pointerId\":0,"
        "    \"pointerType\":0"
        "  }"
        "}";
    m_aplCoreConnectionManager->handleMessage(payload);
}

}  // namespace test
}  // namespace APLClient
