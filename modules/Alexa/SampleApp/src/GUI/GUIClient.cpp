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

#include <vector>

#include <AVSCommon/Utils/JSON/JSONUtils.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include <Utils/SmartScreenSDKVersion.h>
#include "SampleApp/Messages/GUIClientMessage.h"

#include "SampleApp/GUI/GUIClient.h"

static const std::string TAG{"GUIClient"};
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The level json key in the message.
static const std::string LEVEL_TAG("level");

/// The message type for initResponse.
static const std::string MESSAGE_TYPE_INIT_RESPONSE("initResponse");

/// The message type for Tap To Talk.
static const std::string MESSAGE_TYPE_TAP_TO_TALK("tapToTalk");

/// The message type for Hold To Talk start.
static const std::string MESSAGE_TYPE_HOLD_TO_TALK_START("holdToTalkStart");

/// The message type for Hold To Talk end.
static const std::string MESSAGE_TYPE_HOLD_TO_TALK_END("holdToTalkEnd");

/// The message type for Focus acquire request.
static const std::string MESSAGE_TYPE_FOCUS_ACQUIRE_REQUEST("focusAcquireRequest");

/// The message type for Focus release request.
static const std::string MESSAGE_TYPE_FOCUS_RELEASE_REQUEST("focusReleaseRequest");

/// The message type for Focus request.
static const std::string MESSAGE_TYPE_ON_FOCUS_CHANGED_RECEIVED_CONFIRMATION("onFocusChangedReceivedConfirmation");

/// The message type for Static RenderDocument.
static const std::string MESSAGE_TYPE_RENDER_STATIC_DOCUMENT("renderStaticDocument");

/// The message type for ExecuteCommands.
static const std::string MESSAGE_TYPE_EXECUTE_COMMANDS("executeCommands");

/// The message type for ActivityEvent.
static const std::string MESSAGE_TYPE_ACTIVITY_EVENT("activityEvent");

/// The message type for NavigationEvent.
static const std::string MESSAGE_TYPE_NAVIGATION_EVENT("navigationEvent");

/// The message type for APL Core Events.
static const std::string MESSAGE_TYPE_APL_EVENT("aplEvent");

/// The message type for LogEvent.
static const std::string MESSAGE_TYPE_LOG_EVENT("logEvent");

/// The message type for device window state.
static const std::string MESSAGE_TYPE_DEVICE_WINDOW_STATE("deviceWindowState");

/// The message type for device window state.
static const std::string MESSAGE_TYPE_RENDER_COMPLETE("renderComplete");

/// The message type for display metrics event
static const std::string MESSAGE_TYPE_DISPLAY_METRICS("aplDisplayMetrics");

/// The message type for toggling captions.
static const std::string MESSAGE_TYPE_TOGGLE_CAPTIONS("toggleCaptions");

/// The message type for answering a call.
static const std::string MESSAGE_TYPE_ACCEPT_CALL("acceptCall");

/// The message type for hanging up a call.
static const std::string MESSAGE_TYPE_STOP_CALL("stopCall");

/// The message type for enabling local video during a call.
static const std::string MESSAGE_TYPE_ENABLE_LOCAL_VIDEO("enableLocalVideo");

/// The message type for disabling local video during a call.
static const std::string MESSAGE_TYPE_DISABLE_LOCAL_VIDEO("disableLocalVideo");

/// The message type for sending DTMF keys during a PSTN call.
static const std::string MESSAGE_TYPE_SEND_DTMF("sendDtmf");

/// The message type for toggling DoNotDisturb.
static const std::string MESSAGE_TYPE_TOGGLE_DONOTDISTURB("toggleDoNotDisturb");

/// The message type for enabling or disabling camera microphone.
static const std::string MESSAGE_TYPE_SET_CAMERA_MICROPHONE_STATE("setCameraMicrophoneState");

/// The message type for indicating camera first frame rendered.
static const std::string MESSAGE_TYPE_CAMERA_FIRST_FRAME_RENDERED("cameraFirstFrameRendered");

/// Key for isSupported.
static const std::string IS_SUPPORTED_TAG("isSupported");

/// The type json key in the message.
static const std::string TYPE_TAG("type");

/// The component json key in the message.
static const std::string COMPONENT_TAG("component");

/// The message json key in the message.
static const std::string MESSAGE_TAG("message");

/// The payload json key in the message.
static const std::string PAYLOAD_TAG("payload");

/// The token json key in the message.
static const std::string TOKEN_TAG("token");

/// The window id json key in the message.
static const std::string WINDOW_ID_TAG("windowId");

/// The result json key in the message.
static const std::string RESULT_TAG("result");

/// The error json key in the message.
static const std::string ERROR_TAG("error");

/// The event json key in the message.
static const std::string EVENT_TAG("event");

/// The DTMF tone json key in the message.
static const std::string DTMF_TONE_TAG("dtmfTone");

/// The drop frame count json key in the message.
static const std::string DROP_FRAME_COUNT_TAG("dropFrameCount");

/// The payload json key in the message.
static const std::string DEFAULT_WINDOW_ID_TAG("defaultWindowId");

/// The instances json key in the message.
static const std::string INSTANCES_TAG("instances");

/// The id json key in the message.
static const std::string ID_TAG("id");

/// The state json key in the message.
static const std::string ENABLED_TAG("enabled");

/// The key in our config file to find the root of GUI configuration
static const std::string GUI_CONFIGURATION_ROOT_KEY = "gui";

/// The key in our config file to find the root of VisualCharacteristics configuration
static const std::string VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY = "visualCharacteristics";

/// The key in our config file to find the root of app configuration
static const std::string APPCONFIG_CONFIGURATION_ROOT_KEY = "appConfig";

/// The key in our config file to find the optional live view controller ui configuration
static const std::string LIVEVIEWCONTROLLEROPTIONS_CONFIGURATION_ROOT_KEY = "liveViewControllerOptions";

/// The key in our config file to find the root of windows configuration
static const std::string WINDOWS_CONFIGURATION_ROOT_KEY = "windows";

/// The for the window id from window configuration
static const std::string WINDOW_ID_KEY{"id"};

/// The for the supported extension from window configuration
static const std::string SUPPORTED_EXTN_KEY{"supportedExtensions"};

/// APL Window ID for PlayerInfo
static const std::string RENDER_PLAYER_INFO_WINDOW_ID{"renderPlayerInfo"};

/// APL Window ID for LiveView UI
static const std::string LIVE_VIEW_UI_WINDOW_ID{"liveViewUI"};

/// AVS interface json key
static const std::string AVS_INTERFACE_KEY{"avsInterface"};

/// Channel name json key
static const std::string CHANNEL_NAME_KEY{"channelName"};

/// Content type json key
static const std::string CONTENT_TYPE_KEY{"contentType"};

/// Mixable content type id
static const std::string MIXABLE_CONTENT_TYPE_KEY{"MIXABLE"};

/// Nonmixable content type id
static const std::string NONMIXABLE_CONTENT_TYPE_KEY{"NONMIXABLE"};

/// One second Autorelease timeout
static const std::chrono::seconds AUTORELEASE_DURATION{1};

/// Identifier for the document sent in an APL directive
static const std::string DOCUMENT_FIELD = "document";

/// Identifier for the datasources sent in an APL directive
static const std::string DATASOURCES_FIELD = "datasources";

/// Identifier for the supportedViewports array sent in an APL directive
static const std::string SUPPORTED_VIEWPORTS_FIELD = "supportedViewports";

/// Identifier for the presentation object sent in an APL directive
static const std::string PRESENTATION_TOKEN = "presentationToken";

/// Invalid window id runtime error errors key
static const std::string ERRORS_KEY{"errors"};

/// Invalid window id runtime error type key
static const std::string TYPE_KEY{"type"};

/// Invalid window id runtime error reason key
static const std::string REASON_KEY{"reason"};

/// Invalid window id runtime error list id key
static const std::string LIST_ID_KEY{"listId"};

/// Invalid window id runtime error message key
static const std::string MESSAGE_KEY{"message"};

/// Invalid window id runtime error reason
static const std::string INVALID_OPERATION{"INVALID_OPERATION"};

/// Invalid window id runtime error reason
static const std::string INVALID_WINDOW_ID{"Invalid window id"};

/// Invalid window id runtime error message
static const std::string INVALID_WINDOW_ID_MESSAGE{"Device has no window with id: "};

/// Fallback runtime error message
static const std::string FALLBACK_WINDOW_ID_MESSAGE{". Falling back to device default window id: "};

static const std::string DEFAULT_PARAM_VALUE = "{}";

#ifdef ENABLE_COMMS
/// Key for the video calling configuration root in the config file.
static const std::string VIDEO_CALLING_CONFIGURATION_ROOT_KEY = "videoCallingConfig";
#endif

/// Mapping of DTMF enum to characters for Comms dial tones
static const std::map<std::string, alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone>
    DTMF_TONE_STRING_TO_ENUM_MAP = {
        {"0", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_ZERO},
        {"1", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_ONE},
        {"2", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_TWO},
        {"3", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_THREE},
        {"4", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_FOUR},
        {"5", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_FIVE},
        {"6", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_SIX},
        {"7", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_SEVEN},
        {"8", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_EIGHT},
        {"9", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_NINE},
        {"*", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_STAR},
        {"#", alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_POUND},
};

std::string mapDTMFTonetype(alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone) {
    switch (dtmfTone) {
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_ZERO:
            return "0";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_ONE:
            return "1";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_TWO:
            return "2";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_THREE:
            return "3";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_FOUR:
            return "4";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_FIVE:
            return "5";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_SIX:
            return "6";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_SEVEN:
            return "7";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_EIGHT:
            return "8";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_NINE:
            return "9";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_STAR:
            return "*";
        case alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone::DTMF_POUND:
            return "#";
    }
    return "";
}

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace gui {

using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::timing;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::storage;

using namespace smartScreenSDKInterfaces;
using namespace smartScreenCapabilityAgents::alexaPresentation;
using namespace smartScreenCapabilityAgents::templateRuntime;

std::shared_ptr<GUIClient> GUIClient::create(
    std::shared_ptr<MessagingServerInterface> serverImplementation,
    const std::shared_ptr<MiscStorageInterface>& miscStorage,
    const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> customerDataManager) {
    if (!serverImplementation) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullServerImplementation"));
        return nullptr;
    }

    if (!customerDataManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullCustomerDataManager"));
        return nullptr;
    }

    return std::shared_ptr<GUIClient>(new GUIClient(serverImplementation, miscStorage, customerDataManager));
}

GUIClient::GUIClient(
    std::shared_ptr<MessagingServerInterface> serverImplementation,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface>& miscStorage,
    const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> customerDataManager) :
        RequiresShutdown{"GUIClient"},
        CustomerDataHandler{customerDataManager},
        m_serverImplementation{serverImplementation},
        m_hasServerStarted{false},
        m_initMessageReceived{false},
        m_errorState{false},
        m_shouldRestart{false},
        m_limitedInteraction{false},
        m_captionManager{SmartScreenCaptionStateManager(miscStorage)} {
    m_messageHandlers.emplace(
        MESSAGE_TYPE_TAP_TO_TALK, [this](rapidjson::Document& payload) { executeHandleTapToTalk(payload); });
    m_messageHandlers.emplace(MESSAGE_TYPE_HOLD_TO_TALK_START, [this](rapidjson::Document& payload) {
        executeHandleHoldToTalkStart(payload);
    });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_HOLD_TO_TALK_END, [this](rapidjson::Document& payload) { executeHandleHoldToTalkEnd(payload); });
    m_messageHandlers.emplace(MESSAGE_TYPE_FOCUS_ACQUIRE_REQUEST, [this](rapidjson::Document& payload) {
        executeHandleFocusAcquireRequest(payload);
    });
    m_messageHandlers.emplace(MESSAGE_TYPE_FOCUS_RELEASE_REQUEST, [this](rapidjson::Document& payload) {
        executeHandleFocusReleaseRequest(payload);
    });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_ON_FOCUS_CHANGED_RECEIVED_CONFIRMATION,
        [this](rapidjson::Document& payload) { executeHandleOnFocusChangedReceivedConfirmation(payload); });
    m_messageHandlers.emplace(MESSAGE_TYPE_RENDER_STATIC_DOCUMENT, [this](rapidjson::Document& payload) {
        executeHandleRenderStaticDocument(payload);
    });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_EXECUTE_COMMANDS, [this](rapidjson::Document& payload) { executeHandleExecuteCommands(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_ACTIVITY_EVENT, [this](rapidjson::Document& payload) { executeHandleActivityEvent(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_NAVIGATION_EVENT, [this](rapidjson::Document& payload) { executeHandleNavigationEvent(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_APL_EVENT, [this](rapidjson::Document& payload) { executeHandleAplEvent(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_LOG_EVENT, [this](rapidjson::Document& payload) { executeHandleLogEvent(payload); });
    m_messageHandlers.emplace(MESSAGE_TYPE_DEVICE_WINDOW_STATE, [this](rapidjson::Document& payload) {
        executeHandleDeviceWindowState(payload);
    });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_RENDER_COMPLETE, [this](rapidjson::Document& payload) { executeHandleRenderComplete(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_DISPLAY_METRICS, [this](rapidjson::Document& payload) { executeHandleDisplayMetrics(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_TOGGLE_CAPTIONS, [this](rapidjson::Document& payload) { m_captionManager.toggleCaptions(); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_ACCEPT_CALL, [this](rapidjson::Document& payload) { executeHandleAcceptCall(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_STOP_CALL, [this](rapidjson::Document& payload) { executeHandleStopCall(payload); });

    m_messageHandlers.emplace(MESSAGE_TYPE_ENABLE_LOCAL_VIDEO, [this](rapidjson::Document& payload) {
        executeHandleEnableLocalVideo(payload);
    });
    m_messageHandlers.emplace(MESSAGE_TYPE_DISABLE_LOCAL_VIDEO, [this](rapidjson::Document& payload) {
        executeHandleDisableLocalVideo(payload);
    });

    m_messageHandlers.emplace(
        MESSAGE_TYPE_SEND_DTMF, [this](rapidjson::Document& payload) { executeHandleSendDtmf(payload); });

    m_messageHandlers.emplace(MESSAGE_TYPE_TOGGLE_DONOTDISTURB, [this](rapidjson::Document& payload) {
        m_guiManager->handleToggleDoNotDisturbEvent();
    });

#ifdef ENABLE_RTCSC
    m_messageHandlers.emplace(MESSAGE_TYPE_SET_CAMERA_MICROPHONE_STATE, [this](rapidjson::Document& payload) {
        executeSetCameraMicrophoneState(payload);
    });
    m_messageHandlers.emplace(MESSAGE_TYPE_CAMERA_FIRST_FRAME_RENDERED, [this](rapidjson::Document& payload) {
        executeCameraFirstFrameRendered();
    });
#endif

    initGuiConfigs();
}

void GUIClient::doShutdown() {
    ACSDK_DEBUG3(LX(__func__));
    stop();
    m_executor.shutdown();
    m_guiManager.reset();
    m_aplClientBridge.reset();
    m_messageListener.reset();
    m_observer.reset();
    m_serverImplementation.reset();

    std::lock_guard<std::mutex> lock{m_mapMutex};
    m_focusObservers.clear();
}

void GUIClient::setGUIManager(
    std::shared_ptr<alexaSmartScreenSDK::smartScreenSDKInterfaces::GUIServerInterface> guiManager) {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this, guiManager]() {
        if (!m_aplClientBridge) {
            ACSDK_ERROR(LX("setGUIManagerFailed").d("reason", "nullAplRenderer"));
            return;
        }
        m_guiManager = guiManager;
        m_aplClientBridge->setGUIManager(guiManager);
    });
}

void GUIClient::setAplClientBridge(std::shared_ptr<AplClientBridge> aplClientBridge, bool aplVersionChanged) {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this, aplClientBridge, aplVersionChanged]() {
        m_aplClientBridge = aplClientBridge;
        if (aplVersionChanged) {
            m_shouldRestart = true;
        }
        initializeAllRenderers();
    });
}

bool GUIClient::acquireFocus(
    std::string avsInterface,
    std::string channelName,
    alexaClientSDK::avsCommon::avs::ContentType contentType,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    ACSDK_DEBUG5(LX(__func__));

    return m_executor
        .submit([this, avsInterface, channelName, contentType, channelObserver]() {
            return executeAcquireFocus(avsInterface, channelName, contentType, channelObserver);
        })
        .get();
}

bool GUIClient::releaseFocus(
    std::string avsInterface,
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    ACSDK_DEBUG5(LX(__func__));
    return m_executor
        .submit([this, avsInterface, channelName, channelObserver]() {
            return executeReleaseFocus(avsInterface, channelName, channelObserver);
        })
        .get();
}

#ifdef ENABLE_COMMS
void GUIClient::sendCallStateInfo(
    const alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface::CallStateInfo& callStateInfo) {
    ACSDK_DEBUG5(LX(__func__));
    m_executor.submit([this, callStateInfo]() { executeSendCallStateInfo(callStateInfo); });
}

void GUIClient::notifyDtmfTonesSent(
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone>& dtmfTones) {
    ACSDK_DEBUG5(LX(__func__));
    m_executor.submit([this, dtmfTones]() { executeNotifyDtmfTonesSent(dtmfTones); });
}
#endif

bool GUIClient::executeAcquireFocus(
    std::string avsInterface,
    std::string channelName,
    alexaClientSDK::avsCommon::avs::ContentType contentType,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_guiManager->handleFocusAcquireRequest(avsInterface, channelName, contentType, channelObserver);
}

bool GUIClient::executeReleaseFocus(
    std::string avsInterface,
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_guiManager->handleFocusReleaseRequest(avsInterface, channelName, channelObserver);
}

bool GUIClient::isReady() {
    return m_hasServerStarted && m_initMessageReceived && !m_errorState;
}

void GUIClient::setMessageListener(std::shared_ptr<MessageListenerInterface> messageListener) {
    m_executor.submit([this, messageListener]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_messageListener = messageListener;
    });
}

bool GUIClient::start() {
    m_executor.submit([this]() {
        // start the server asynchronously.
        m_serverThread = std::thread(&GUIClient::serverThread, this);
    });

    return true;
};

void GUIClient::serverThread() {
    ACSDK_DEBUG3(LX("serverThread"));
    if (m_serverImplementation) {
        m_serverImplementation->setMessageListener(shared_from_this());
        m_serverImplementation->setObserver(shared_from_this());

        m_hasServerStarted = true;

        if (!m_serverImplementation->start()) {
            m_hasServerStarted = false;
            m_errorState = true;
            ACSDK_ERROR(LX("serverThreadFailed").d("reason", "start failed"));
            return;
        }
    } else {
        ACSDK_ERROR(LX("serverThreadFailed").d("reason", "noServerImplementation"));
    }
}

void GUIClient::stop() {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this]() {
        if (m_hasServerStarted) {
            m_serverImplementation->stop();
        }
        m_hasServerStarted = m_initMessageReceived = m_errorState = false;
    });
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
}

void GUIClient::onMessage(const std::string& jsonPayload) {
    m_executor.submit([this, jsonPayload]() {
        ACSDK_DEBUG9(LX("onMessageInExector").d("payload", jsonPayload));
        rapidjson::Document message;
        rapidjson::ParseResult result = message.Parse(jsonPayload);
        if (!result) {
            ACSDK_ERROR(LX("onMessageFailed").d("reason", "parsingPayloadFailed").d("message", jsonPayload));
            return;
        }

        if (m_messageListener) {
            m_messageListener->onMessage(jsonPayload);
        }

        std::string messageType;
        if (!jsonUtils::retrieveValue(message, TYPE_TAG, &messageType)) {
            ACSDK_ERROR(LX("onMessageFailed").d("reason", "typeNotFound").sensitive("message", jsonPayload));
            return;
        }

        if (MESSAGE_TYPE_INIT_RESPONSE == messageType) {
            executeProcessInitResponse(message);
        } else {
            auto messageHandler = m_messageHandlers.find(messageType);
            if (messageHandler != m_messageHandlers.end()) {
                messageHandler->second(message);
            } else {
                ACSDK_WARN(LX("onMessageFailed").d("reason", "unknownType").d("type", messageType));
            }
        }
    });
}

void GUIClient::executeCommands(const std::string& command, const std::string& token) {
    m_executor.submit([this, command, token]() { m_aplClientBridge->executeCommands(command, token); });
}

void GUIClient::dataSourceUpdate(
    const std::string& sourceType,
    const std::string& jsonPayload,
    const std::string& token) {
    m_executor.submit([this, sourceType, jsonPayload, token]() {
        m_aplClientBridge->dataSourceUpdate(sourceType, jsonPayload, token);
    });
}

void GUIClient::provideState(const std::string& aplToken, const unsigned int stateRequestToken) {
    m_executor.submit(
        [this, aplToken, stateRequestToken]() { m_aplClientBridge->provideState(aplToken, stateRequestToken); });
}

void GUIClient::interruptCommandSequence(const std::string& token) {
    m_guiManager->onUserEvent();
    m_executor.submit([this, token]() { m_aplClientBridge->interruptCommandSequence(token); });
}

void GUIClient::onPresentationSessionChanged(
    const std::string& id,
    const std::string& skillId,
    const std::vector<smartScreenSDKInterfaces::GrantedExtension>& grantedExtensions,
    const std::vector<smartScreenSDKInterfaces::AutoInitializedExtension>& autoInitializedExtensions) {
    m_executor.submit([this, id, skillId]() { m_aplClientBridge->onPresentationSessionChanged(id, skillId); });
}

void GUIClient::executeHandleTapToTalk(rapidjson::Document& message) {
    m_guiManager->handleTapToTalk();
}

void GUIClient::executeHandleHoldToTalkStart(rapidjson::Document& message) {
    m_guiManager->handleHoldToTalk(true);
}

void GUIClient::executeHandleHoldToTalkEnd(rapidjson::Document& message) {
    m_guiManager->handleHoldToTalk(false);
}

void GUIClient::executeHandleAcceptCall(rapidjson::Document& message) {
    m_guiManager->acceptCall();
}

void GUIClient::executeHandleStopCall(rapidjson::Document& message) {
    m_guiManager->stopCall();
}

void GUIClient::executeHandleEnableLocalVideo(rapidjson::Document& message) {
    m_guiManager->enableLocalVideo();
}

void GUIClient::executeHandleDisableLocalVideo(rapidjson::Document& message) {
    m_guiManager->disableLocalVideo();
}

#ifdef ENABLE_RTCSC
void GUIClient::executeSetCameraMicrophoneState(rapidjson::Document& message) {
    ACSDK_DEBUG5(LX(__func__));

    if (!message.HasMember(ENABLED_TAG)) {
        ACSDK_ERROR(LX("setCameraMicrophoneStateFailed").d("reason", "json payload does not contain enabled"));
        return;
    }

    if (!message[ENABLED_TAG].IsBool()) {
        ACSDK_ERROR(LX("setCameraMicrophoneStateFailed").d("reason", "enabled is not boolean"));
        return;
    }

    bool state{message[ENABLED_TAG].GetBool()};
    m_guiManager->handleSetCameraMicrophoneState(state);
}

void GUIClient::executeCameraFirstFrameRendered() {
    ACSDK_DEBUG5(LX(__func__));
    if (m_aplLiveViewExtension) {
        m_aplLiveViewExtension->onCameraFirstFrameRendered();
    }
}
#endif

void GUIClient::executeHandleSendDtmf(rapidjson::Document& message) {
    std::string dtmfString;
    if (!jsonUtils::retrieveValue(message, DTMF_TONE_TAG, &dtmfString)) {
        ACSDK_ERROR(LX("handleSendDtmfRequestFailed").d("reason", "dtmfToneNotFound"));
        return;
    }
    ACSDK_DEBUG3(LX("handleSendDtmfRequest").d(DTMF_TONE_TAG, dtmfString));

    auto dtmfIterator = DTMF_TONE_STRING_TO_ENUM_MAP.find(dtmfString);
    if (dtmfIterator == DTMF_TONE_STRING_TO_ENUM_MAP.end()) {
        ACSDK_ERROR(LX("handleSendDtmfRequestFailed").d("unknown dtmfTone", dtmfString));
        return;
    }
    alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone dtmfTone = dtmfIterator->second;
    m_guiManager->sendDtmf(dtmfTone);
}

void GUIClient::executeHandleFocusAcquireRequest(rapidjson::Document& message) {
    ACSDK_CRITICAL(LX(__func__));
    std::string avsInterface;
    if (!jsonUtils::retrieveValue(message, AVS_INTERFACE_KEY, &avsInterface)) {
        ACSDK_ERROR(LX("handleFocusAcquireRequestFailed").d("reason", "avsInterfaceNotFound"));
        return;
    }

    GUIToken token = 0;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleFocusAcquireRequestFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string channelName;
    if (!jsonUtils::retrieveValue(message, CHANNEL_NAME_KEY, &channelName)) {
        ACSDK_ERROR(LX("handleFocusAcquireRequestFailed").d("reason", "channelNameNotFound"));
        return;
    }

    alexaClientSDK::avsCommon::avs::ContentType contentType = alexaClientSDK::avsCommon::avs::ContentType::UNDEFINED;
    std::string contentTypeStr;
    if (!jsonUtils::retrieveValue(message, CONTENT_TYPE_KEY, &contentTypeStr)) {
        ACSDK_WARN(LX(__func__).d("reason", "contentTypeUndefined"));
    } else {
        if (contentTypeStr == MIXABLE_CONTENT_TYPE_KEY) {
            contentType = alexaClientSDK::avsCommon::avs::ContentType::MIXABLE;
        } else if (contentTypeStr == NONMIXABLE_CONTENT_TYPE_KEY) {
            contentType = alexaClientSDK::avsCommon::avs::ContentType::NONMIXABLE;
        } else {
            ACSDK_WARN(LX(__func__).d("reason", "contentTypeInvalid").d("contentType", contentTypeStr));
        }
    }

    executeFocusAcquireRequest(token, avsInterface, channelName, contentType);
}

void GUIClient::executeHandleLogEvent(rapidjson::Document& message) {
    std::string level;
    if (!jsonUtils::retrieveValue(message, LEVEL_TAG, &level)) {
        ACSDK_ERROR(LX("handleLogEventFailed").d("reason", "levelNotFound"));
        return;
    }

    std::string component;
    if (!jsonUtils::retrieveValue(message, COMPONENT_TAG, &component)) {
        ACSDK_ERROR(LX("handleLogEventFailed").d("reason", "componentNotFound"));
        return;
    }

    std::string logMessage;
    if (!jsonUtils::retrieveValue(message, MESSAGE_TAG, &logMessage)) {
        ACSDK_ERROR(LX("handleLogEventFailed").d("reason", "messageNotFound"));
        return;
    }

    m_rendererLogBridge.log(level, component, logMessage);
}

void GUIClient::executeFocusAcquireRequest(
    const GUIToken token,
    const std::string& avsInterface,
    const std::string& channelName,
    alexaClientSDK::avsCommon::avs::ContentType contentType) {
    bool result = true;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> focusObserver;
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        if (m_focusObservers.count(token) == 0) {
            m_focusObservers[token] =
                std::make_shared<ProxyFocusObserver>(avsInterface, token, shared_from_this(), channelName);
            focusObserver = m_focusObservers[token];
        } else {
            result = false;
        }
    }

    if (!result) {
        ACSDK_ERROR(LX("executeFocusAcquireRequestFail").d("token", token).d("reason", "observer already exists"));
        executeSendFocusResponse(token, false);
        return;
    }

    result = executeAcquireFocus(avsInterface, channelName, contentType, focusObserver);
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusAcquireRequestFail").d("token", token).d("reason", "acquireChannel returned false"));
        executeSendFocusResponse(token, false);
        return;
    }

    executeSendFocusResponse(token, true);
}

void GUIClient::executeHandleFocusReleaseRequest(rapidjson::Document& message) {
    std::string avsInterface;
    if (!jsonUtils::retrieveValue(message, AVS_INTERFACE_KEY, &avsInterface)) {
        ACSDK_ERROR(LX("handleFocusReleaseRequestFailed").d("reason", "avsInterfaceNotFound"));
        return;
    }

    GUIToken token = 0;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleFocusReleaseRequestFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string channelName;
    if (!jsonUtils::retrieveValue(message, CHANNEL_NAME_KEY, &channelName)) {
        ACSDK_ERROR(LX("handleFocusReleaseRequestFailed").d("reason", "channelNameNotFound"));
        return;
    }

    executeFocusReleaseRequest(token, avsInterface, channelName);
}

void GUIClient::executeFocusReleaseRequest(
    const GUIToken token,
    const std::string& avsInterface,
    const std::string& channelName) {
    bool result = true;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> focusObserver;
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        auto it = m_focusObservers.find(token);
        if (it == m_focusObservers.end()) {
            result = false;
        } else {
            focusObserver = it->second;
        }
    }

    if (!result || !focusObserver) {
        ACSDK_ERROR(LX("executeFocusReleaseRequestFail").d("token", token).d("reason", "no observer found"));
        executeSendFocusResponse(token, false);
        return;
    }

    result = executeReleaseFocus(avsInterface, channelName, focusObserver);
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusReleaseRequestFail").d("token", token).d("reason", "releaseChannel returned false"));
        executeSendFocusResponse(token, false);
        return;
    }
    executeSendFocusResponse(token, true);
}

void GUIClient::executeSendFocusResponse(const GUIToken token, const bool result) {
    auto message = messages::FocusResponseMessage(token, result);
    sendMessage(message);
}

void GUIClient::executeHandleOnFocusChangedReceivedConfirmation(rapidjson::Document& message) {
    GUIToken token = 0;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleOnFocusChangedReceivedConfirmationFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::lock_guard<std::mutex> lock{m_mapMutex};
    auto currentAutoReleaseTimer = m_autoReleaseTimers.find(token);
    if (currentAutoReleaseTimer != m_autoReleaseTimers.end()) {
        if (!currentAutoReleaseTimer->second) {
            ACSDK_ERROR(LX("processOnFocusChangedReceivedConfirmationFail")
                            .d("token", token)
                            .d("reason", "autoReleaseTimer is null"));
            return;
        }
        currentAutoReleaseTimer->second->stop();
    }
}

void GUIClient::executeHandleRenderStaticDocument(rapidjson::Document& message) {
    std::string token;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleRenderStaticDocumentFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string payload;
    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &payload)) {
        ACSDK_ERROR(LX("handleRenderStaticDocumentFailed").d("reason", "payloadNotFound"));
        return;
    }

    std::string windowId;
    if (!jsonUtils::retrieveValue(message, WINDOW_ID_TAG, &windowId)) {
        ACSDK_ERROR(LX("handleRenderStaticDocumentFailed").d("reason", "windowIdNotFound"));
        return;
    }

    std::string document = extractDocument(payload);
    std::string datasources = extractDatasources(payload);
    std::string supportedViewports = extractSupportedViewports(payload);

    m_aplClientBridge->renderDocument(token, document, datasources, supportedViewports, windowId);
}

void GUIClient::executeHandleExecuteCommands(rapidjson::Document& message) {
    std::string token;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleExecuteCommandsFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string payload;
    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &payload)) {
        ACSDK_ERROR(LX("handleExecuteCommandsFailed").d("reason", "payloadNotFound"));
        return;
    }

    m_aplClientBridge->executeCommands(payload, token);
}

void GUIClient::onRenderDirectiveReceived(
    const std::string& token,
    const std::chrono::steady_clock::time_point& receiveTime) {
    m_aplClientBridge->onRenderDirectiveReceived(token, receiveTime);
}

void GUIClient::onRenderingAborted(const std::string& token) {
    m_aplClientBridge->handleRenderingEvent(token, APLClient::AplRenderingEvent::RENDER_ABORTED);
}

void GUIClient::onMetricRecorderAvailable(
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    m_aplClientBridge->onMetricRecorderAvailable(metricRecorder);
}

void GUIClient::executeHandleActivityEvent(rapidjson::Document& message) {
    ACSDK_DEBUG5(LX("executeHandleActivityEvent"));

    std::string event;
    if (!jsonUtils::retrieveValue(message, EVENT_TAG, &event)) {
        ACSDK_ERROR(LX("handleActivityEventFailed").d("reason", "eventNotFound"));
        return;
    }
    smartScreenSDKInterfaces::ActivityEvent activityEvent = smartScreenSDKInterfaces::activityEventFromString(event);
    if (smartScreenSDKInterfaces::ActivityEvent::UNKNOWN == activityEvent) {
        ACSDK_ERROR(LX("handleActivityEventFailed").d("reason", "received unknown type of event"));
        return;
    }

    m_guiManager->handleActivityEvent(activityEvent);
}

void GUIClient::executeHandleNavigationEvent(rapidjson::Document& message) {
    std::string event;
    if (!jsonUtils::retrieveValue(message, EVENT_TAG, &event)) {
        ACSDK_ERROR(LX("handleNavigationEventFailed").d("reason", "eventNotFound"));
        return;
    }
    smartScreenSDKInterfaces::NavigationEvent navigationEvent =
        smartScreenSDKInterfaces::navigationEventFromString(event);
    if (smartScreenSDKInterfaces::NavigationEvent::UNKNOWN == navigationEvent) {
        ACSDK_ERROR(LX("handleNavigationEventFailed").d("reason", "received unknown type of event"));
        return;
    }

    m_guiManager->handleNavigationEvent(navigationEvent);
}

void GUIClient::executeHandleAplEvent(rapidjson::Document& message) {
    if (!m_aplClientBridge) {
        ACSDK_ERROR(LX("handleAplEventFailed").d("reason", "APL Renderer has not been configured"));
        return;
    }

    std::string payload;
    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &payload)) {
        ACSDK_ERROR(LX("handleAplEventFailed").d("reason", "payloadNotFound"));
        return;
    }

    std::string windowId;
    if (!jsonUtils::retrieveValue(message, WINDOW_ID_TAG, &windowId)) {
        ACSDK_ERROR(LX("handleAplEventFailed").d("reason", "windowIdNotFound"));
        return;
    }

    m_aplClientBridge->onMessage(windowId, payload);
}

void GUIClient::executeHandleDeviceWindowState(rapidjson::Document& message) {
    std::string payload;
    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &payload)) {
        ACSDK_ERROR(LX("handleDeviceWindowStateFailed").d("reason", "payloadValueNotFound"));
        return;
    }

    const auto& jsonPayload = message[PAYLOAD_TAG];
    if (!jsonPayload.IsObject()) {
        ACSDK_ERROR(LX("handleDeviceWindowStateFailed").d("reason", "payloadObjectNotFound"));
        return;
    }

    if (!jsonUtils::retrieveValue(jsonPayload, DEFAULT_WINDOW_ID_TAG, &m_defaultWindowId)) {
        ACSDK_ERROR(LX("handleDeviceWindowStateFailed").d("reason", "defaultWindowIdNotFound"));
        return;
    }

    const auto& instances = jsonPayload[INSTANCES_TAG];
    if (!instances.IsArray()) {
        ACSDK_ERROR(LX("handleDeviceWindowStateFailed").d("reason", "unableToFindWindowInstances"));
        return;
    }

    m_reportedWindowIds.clear();
    for (rapidjson::SizeType idx = 0; idx < instances.Size(); ++idx) {
        m_reportedWindowIds.insert(instances[idx][ID_TAG].GetString());
    }

    m_guiManager->handleDeviceWindowState(payload);
}

void GUIClient::executeHandleRenderComplete(rapidjson::Document& message) {
    std::string windowId;
    if (!jsonUtils::retrieveValue(message, WINDOW_ID_TAG, &windowId)) {
        ACSDK_ERROR(LX("executeHandleRenderComplete").d("reason", "windowIdNotFound"));
        return;
    }

    m_guiManager->handleRenderComplete();
    m_aplClientBridge->handleRenderingEvent(windowId, APLClient::AplRenderingEvent::DOCUMENT_RENDERED);
}

void GUIClient::executeHandleDisplayMetrics(rapidjson::Document& message) {
    std::string windowId;
    std::string jsonPayload;

    if (!jsonUtils::retrieveValue(message, WINDOW_ID_TAG, &windowId)) {
        ACSDK_ERROR(LX("executeHandleDisplayMetricsFailed").d("reason", "windowIdNotFound"));
        return;
    }

    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &jsonPayload)) {
        ACSDK_ERROR(LX("executeHandleDisplayMetricsFailed").d("reason", "payloadNotFound"));
        return;
    }

    m_aplClientBridge->handleDisplayMetrics(windowId, jsonPayload);
}

void GUIClient::setObserver(const std::shared_ptr<MessagingServerObserverInterface>& observer) {
    m_executor.submit([this, observer]() { m_observer = observer; });
}

void GUIClient::onConnectionOpened() {
    ACSDK_DEBUG3(LX("onConnectionOpened"));
    m_executor.submit([this]() {
        if (!m_initThread.joinable()) {
            m_initThread = std::thread(&GUIClient::sendInitRequestAndWait, this);
        } else {
            ACSDK_INFO(LX("onConnectionOpened").m("init thread is not avilable"));
        }

        if (m_observer) {
            m_observer->onConnectionOpened();
        }
        m_guiManager->handleOnMessagingServerConnectionOpened();
    });
}

void GUIClient::onConnectionClosed() {
    ACSDK_DEBUG3(LX("onConnectionClosed"));
    m_executor.submit([this]() {
        if (!m_serverImplementation->isReady()) {
            m_initMessageReceived = false;
        }

        if (m_initThread.joinable()) {
            m_initThread.join();
        }

        if (m_observer) {
            m_observer->onConnectionClosed();
        }
        m_aplClientBridge->onConnectionClosed();
    });
}

void GUIClient::renderTemplateCard(
    const std::string& token,
    const std::string& jsonPayload,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    auto message = messages::RenderTemplateMessage(token, jsonPayload);
    sendMessage(message);
}

void GUIClient::clearTemplateCard(const std::string& token) {
    ACSDK_DEBUG5(LX("clearTemplateCard"));
    m_executor.submit([this, token]() { m_aplClientBridge->clearDocument(token); });
}

void GUIClient::renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) {
    m_executor.submit([this, jsonPayload, windowId, token]() {
        bool isWindowIdPresent = m_reportedWindowIds.find(windowId) != m_reportedWindowIds.end();

        std::string document = extractDocument(jsonPayload);
        std::string datasources = extractDatasources(jsonPayload);
        std::string supportedViewports = extractSupportedViewports(jsonPayload);
        std::string targetWindowId = isWindowIdPresent ? windowId : m_defaultWindowId;

        m_aplClientBridge->renderDocument(token, document, datasources, supportedViewports, targetWindowId);
        if (!isWindowIdPresent && !windowId.empty()) {
            std::ostringstream ossFormattedMessage;
            ossFormattedMessage << INVALID_WINDOW_ID_MESSAGE << windowId << FALLBACK_WINDOW_ID_MESSAGE
                                << m_defaultWindowId;
            reportInvalidWindowIdRuntimeError(ossFormattedMessage.str(), token);
        }
    });
}

void GUIClient::clearDocument(const std::string& token) {
    ACSDK_DEBUG5(LX(__func__).d("token", token));
    m_executor.submit([this, token]() { m_aplClientBridge->clearDocument(token); });
}

void GUIClient::clearData() {
    ACSDK_DEBUG5(LX(__func__));
}

void GUIClient::renderPlayerInfoCard(
    const std::string& token,
    const std::string& jsonPayload,
    smartScreenSDKInterfaces::AudioPlayerInfo info,
    alexaClientSDK::avsCommon::avs::FocusState focusState,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> mediaProperties) {
    auto message = messages::RenderPlayerInfoMessage(token, jsonPayload, info);
    sendMessage(message);
}

void GUIClient::clearPlayerInfoCard(const std::string& token) {
    ACSDK_DEBUG5(LX("clearPlayerInfoCard"));
    m_aplClientBridge->clearDocument(token);

    auto message = messages::ClearPlayerInfoCardMessage();
    sendMessage(message);
}

void GUIClient::renderCaptions(const std::string& payload) {
    if (m_captionManager.areCaptionsEnabled()) {
        ACSDK_DEBUG5(LX("renderCaptions"));

        auto message = messages::RenderCaptionsMessage(payload);
        sendMessage(message);
    }
}

void GUIClient::onDoNotDisturbSettingChanged(bool enable) {
    ACSDK_DEBUG5(LX(__func__));
    auto message = messages::DoNotDisturbSettingChangedMessage(enable);
    sendMessage(message);
}

bool GUIClient::handleNavigationEvent(alexaSmartScreenSDK::smartScreenSDKInterfaces::NavigationEvent event) {
    if (smartScreenSDKInterfaces::NavigationEvent::BACK == event) {
        return m_aplClientBridge->handleBack();
    }
    return false;
}

void GUIClient::handleASRProfileChanged(alexaClientSDK::capabilityAgents::aip::ASRProfile asrProfile) {
#ifdef ENABLE_RTCSC
    if (m_aplLiveViewExtension) {
        m_executor.submit(
            [this, asrProfile]() { m_aplLiveViewExtension->setAsrProfile(asrProfileToString(asrProfile)); });
    }
#endif
}

#ifdef ENABLE_RTCSC
void GUIClient::handleCameraMicrophoneStateChanged(bool enabled) {
    if (m_aplLiveViewExtension) {
        m_aplLiveViewExtension->setCameraMicrophoneState(enabled);
    }
}

void GUIClient::handleChangeCameraMicStateRequest(bool enabled) {
    m_executor.submit([this, enabled]() { m_guiManager->handleSetCameraMicrophoneState(enabled); });
}

void GUIClient::handleCameraExitRequest() {
    m_executor.submit([this]() { m_guiManager->handleClearLiveView(); });
}
#endif

void GUIClient::onLogout() {
    m_shouldRestart = true;
    m_cond.notify_all();
}

SampleAppReturnCode GUIClient::run() {
    ACSDK_DEBUG3(LX("run"));
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock, [this] { return m_shouldRestart || m_errorState; });
    ACSDK_DEBUG3(LX("runExits").d("reason", m_shouldRestart ? "loggedout" : "not initialized"));
    return m_shouldRestart ? SampleAppReturnCode::RESTART
                           : (m_errorState ? SampleAppReturnCode::ERROR : SampleAppReturnCode::OK);
}

void GUIClient::sendInitRequestAndWait() {
    // Wait for server to be ready
    ACSDK_DEBUG9(LX("sendInitRequestAndWait").m("waiting for server to be ready"));
    while (!m_serverImplementation->isReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Send init request message.
    auto message = messages::InitRequestMessage(alexaSmartScreenSDK::utils::smartScreenSDKVersion::getCurrentVersion());
    sendMessage(message);

    // Wait for response
    std::unique_lock<std::mutex> lock(m_mutex);
    ACSDK_DEBUG3(LX("start").m("waiting for InitResponse"));
    m_cond.wait(lock, [this] {
        ACSDK_DEBUG9(LX("sendInitRequestAndWait")
                         .d("errorState", m_errorState)
                         .d("initMessage received", m_initMessageReceived));
        return m_errorState || m_initMessageReceived;
    });

    ACSDK_DEBUG3(LX("start").m("InitResponse received"));
    m_aplClientBridge->onConnectionOpened();
}
void GUIClient::initGuiConfigs() {
    /// Get the root ConfigurationNode.
    auto configurationRoot = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();

    /// Get the root of GUI ConfigurationNode.
    auto configurationGui = configurationRoot[GUI_CONFIGURATION_ROOT_KEY];

    /// Get the ConfigurationNode contains visualCharacteristics config array.
    m_visualCharacteristics = configurationGui.getArray(VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY);

    /// Get the ConfigurationNode contains appConfig.
    m_guiAppConfig = configurationGui[APPCONFIG_CONFIGURATION_ROOT_KEY];

#ifdef ENABLE_RTCSC
    m_liveViewControllerOptionsConfig = configurationGui[LIVEVIEWCONTROLLEROPTIONS_CONFIGURATION_ROOT_KEY];
#endif
}

void GUIClient::executeSendGuiConfiguration() {
    ACSDK_DEBUG9(LX(__func__));

    auto appConfigString = m_guiAppConfig.serialize();
    auto visualCharacteristicsString = m_visualCharacteristics.serialize();

#ifndef _MSC_VER
    auto message = messages::GuiConfigurationMessage(visualCharacteristicsString, appConfigString);
    sendMessage(message);
#else
    auto payloadWithHeader = R"({"type": "guiConfiguration", "payload": {"visualCharacteristics": )" +
                             visualCharacteristicsString + R"(, "appConfig": )" + appConfigString + R"(}})";

    writeMessage(payloadWithHeader);
#endif

#ifdef ENABLE_COMMS
    executeSendVideoCallingConfig();
#endif
}

#ifdef ENABLE_COMMS
void GUIClient::executeSendCallStateInfo(
    const alexaClientSDK::avsCommon::sdkInterfaces::CallStateObserverInterface::CallStateInfo& callStateInfo) {
    auto message = messages::CallStateChangeMessage(callStateInfo);
    sendMessage(message);
}

void GUIClient::executeSendVideoCallingConfig() {
    /// Get the root ConfigurationNode.
    auto configurationRoot = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();
    if (configurationRoot) {
        /// Get videoCallingConfig node
        auto videoCallingConfigRoot = configurationRoot[VIDEO_CALLING_CONFIGURATION_ROOT_KEY];
        if (videoCallingConfigRoot) {
            auto message = messages::VideoCallingConfigMessage(videoCallingConfigRoot.serialize());
            sendMessage(message);
        }
    }
}

void GUIClient::executeNotifyDtmfTonesSent(
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::CallManagerInterface::DTMFTone>& dtmfTones) {
    std::string dtmfTonesString;
    auto it = dtmfTones.begin();
    for (; it != dtmfTones.end(); it++) {
        dtmfTonesString.append(mapDTMFTonetype(*it));
    }
    auto message = messages::DtmfTonesSentMessage(dtmfTonesString);
    sendMessage(message);
}

#endif

bool GUIClient::executeProcessInitResponse(const rapidjson::Document& message) {
    bool isSupported = false;
    if (!jsonUtils::retrieveValue(message, IS_SUPPORTED_TAG, &isSupported)) {
        ACSDK_ERROR(LX("processInitResponseFailed").d("reason", "isSupportedNotFound"));
        m_errorState = true;

        return false;
    }

    if (!isSupported) {
        ACSDK_ERROR(LX("processInitResponseFailed")
                        .d("reason", "Not Supported SDK")
                        .d("SDKVersion", alexaClientSDK::avsCommon::utils::sdkVersion::getCurrentVersion()));

        // Don't get into error state, so GUI client can connect with supported version.
        return false;
    }

    m_initMessageReceived = true;
    m_cond.notify_all();
    if (m_initThread.joinable()) {
        m_initThread.join();
    }
    executeSendGuiConfiguration();
    // Init locale for gui layer after we've initialized
    m_guiManager->handleLocaleChange();
    return true;
}

void GUIClient::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
    m_limitedInteraction = m_limitedInteraction || (newState == AuthObserverInterface::State::UNRECOVERABLE_ERROR);
}

void GUIClient::onCapabilitiesStateChange(
    alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State newState,
    alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::Error newError,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& addedOrUpdatedEndpoints,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& deletedEndpoints) {
    m_limitedInteraction =
        m_limitedInteraction ||
        (newState == alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesObserverInterface::State::FATAL_ERROR);
}

GUIClient::ProxyFocusObserver::ProxyFocusObserver(
    std::string avsInterface,
    const GUIToken token,
    std::shared_ptr<GUIClient> guiClient,
    std::string channelName) :
        m_avsInterface{std::move(avsInterface)},
        m_token{token},
        m_focusBridge{std::move(guiClient)},
        m_channelName{std::move(channelName)} {
}

void GUIClient::ProxyFocusObserver::onFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newFocus,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    if (newFocus != alexaClientSDK::avsCommon::avs::FocusState::NONE) {
        m_focusBridge->startAutoreleaseTimer(m_avsInterface, m_token, m_channelName);
    }
    m_focusBridge->sendOnFocusChanged(m_token, newFocus);
}

void GUIClient::startAutoreleaseTimer(
    const std::string& avsInterface,
    const GUIToken token,
    const std::string& channelName) {
    std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::Timer> timer =
        std::make_shared<alexaClientSDK::avsCommon::utils::timing::Timer>();
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        m_autoReleaseTimers[token] = timer;
    }

    timer->start(AUTORELEASE_DURATION, [this, avsInterface, token, channelName] {
        autoRelease(avsInterface, token, channelName);
    });
}

void GUIClient::autoRelease(const std::string& avsInterface, const GUIToken token, const std::string& channelName) {
    ACSDK_DEBUG5(LX("autoRelease").d("token", token).d("channelName", channelName));
    m_executor.submit([this, avsInterface, token, channelName]() {
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> focusObserver;
        std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::Timer> autoReleaseTimer;
        {
            std::lock_guard<std::mutex> lock{m_mapMutex};
            focusObserver = m_focusObservers[token];
            if (!focusObserver) {
                ACSDK_CRITICAL(LX("autoReleaseFailed").d("token", token).d("reason", "focusObserver is null"));
                return;
            }
        }
        m_guiManager->handleFocusReleaseRequest(avsInterface, channelName, focusObserver);
    });
}

void GUIClient::sendOnFocusChanged(const GUIToken token, const alexaClientSDK::avsCommon::avs::FocusState state) {
    auto message = messages::FocusChangedMessage(token, state);
    sendMessage(message);

    if (state == alexaClientSDK::avsCommon::avs::FocusState::NONE) {
        // Remove observer and timer when released.
        std::lock_guard<std::mutex> lock{m_mapMutex};
        if (m_focusObservers.erase(token) == 0) {
            ACSDK_WARN(LX("sendOnFocusChanged").d("reason", "tokenNotFoundWhenRemovingObserver").d("token", token));
        }
        if (m_autoReleaseTimers.erase(token) == 0) {
            ACSDK_WARN(
                LX("sendOnFocusChanged").d("reason", "tokenNotFoundWhenRemovingAutoReleaseTimer").d("token", token));
        }
    }
}

void GUIClient::sendMessage(smartScreenSDKInterfaces::MessageInterface& message) {
    writeMessage(message.get());
}

void GUIClient::executeSendMessage(smartScreenSDKInterfaces::MessageInterface& message) {
    executeWriteMessage(message.get());
}

void GUIClient::writeMessage(const std::string& payload) {
    m_executor.submit([this, payload]() { executeWriteMessage(payload); });
}

void GUIClient::executeWriteMessage(const std::string& payload) {
    m_serverImplementation->writeMessage(payload);
}

std::string GUIClient::extractDocument(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string aplDocument;
    if (!jsonUtils::retrieveValue(document, DOCUMENT_FIELD, &aplDocument)) {
        aplDocument = DEFAULT_PARAM_VALUE;
    }

    return aplDocument;
}

std::string GUIClient::extractDatasources(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string aplData;
    if (!jsonUtils::retrieveValue(document, DATASOURCES_FIELD, &aplData)) {
        aplData = DEFAULT_PARAM_VALUE;
    }

    return aplData;
}

std::string GUIClient::extractSupportedViewports(const std::string& jsonPayload) {
    rapidjson::Document document;
    document.Parse(jsonPayload);

    std::string supportedViewports;
    rapidjson::Value::ConstMemberIterator jsonIt;
    if (!jsonUtils::findNode(document, SUPPORTED_VIEWPORTS_FIELD, &jsonIt)) {
        supportedViewports = DEFAULT_PARAM_VALUE;
    } else {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        jsonIt->value.Accept(writer);
        supportedViewports = sb.GetString();
    }

    return supportedViewports;
}

void GUIClient::initializeAllRenderers() {
    ACSDK_DEBUG9(LX(__func__));
    auto windowsConfiguration = m_guiAppConfig.getArray(WINDOWS_CONFIGURATION_ROOT_KEY);
    if (windowsConfiguration) {
        for (std::size_t i = 0; i < windowsConfiguration.getArraySize(); ++i) {
            std::string windowId;
            std::set<std::string> supportedExtensions;
            if (!windowsConfiguration[i].getString(WINDOW_ID_KEY, &windowId)) {
                ACSDK_ERROR(LX(__func__).d("incorrectWindowConfiguration", "id not found"));
                continue;
            }

            ACSDK_DEBUG1(LX(__func__).d("initializingWindow", windowId));
            windowsConfiguration[i].getStringValues(SUPPORTED_EXTN_KEY, &supportedExtensions);
            m_aplClientBridge->initializeRenderer(windowId, supportedExtensions);
        }
    };

    // Create PlayerInfo APL Renderer
    m_aplClientBridge->initializeRenderer(RENDER_PLAYER_INFO_WINDOW_ID, {APLClient::Extensions::AudioPlayer::URI});

#ifdef ENABLE_RTCSC
    // Init LiveView extension and create renderer instance that uses it
    m_aplLiveViewExtension = std::make_shared<Extensions::LiveView::AplLiveViewExtension>(shared_from_this());
    m_aplClientBridge->initializeRenderer(LIVE_VIEW_UI_WINDOW_ID, {m_aplLiveViewExtension});
#endif
}

void GUIClient::reportInvalidWindowIdRuntimeError(const std::string& errorMessage, const std::string& aplToken) {
    rapidjson::Document runtimeErrorPayloadJson(rapidjson::kObjectType);
    auto& alloc = runtimeErrorPayloadJson.GetAllocator();
    rapidjson::Value payload(rapidjson::kObjectType);
    rapidjson::Value errors(rapidjson::kArrayType);
    payload.AddMember(rapidjson::StringRef(PRESENTATION_TOKEN), aplToken, alloc);

    rapidjson::Value error(rapidjson::kObjectType);
    error.AddMember(rapidjson::StringRef(TYPE_KEY), INVALID_OPERATION, alloc);
    error.AddMember(rapidjson::StringRef(REASON_KEY), INVALID_WINDOW_ID, alloc);
    error.AddMember(rapidjson::StringRef(LIST_ID_KEY), "", alloc);
    error.AddMember(rapidjson::StringRef(MESSAGE_KEY), errorMessage, alloc);

    errors.PushBack(error, alloc);
    payload.AddMember(rapidjson::StringRef(ERRORS_KEY), errors, alloc);

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    payload.Accept(writer);

    m_guiManager->handleRuntimeErrorEvent(aplToken, sb.GetString());
    ACSDK_WARN(LX("reportInvalidWindowIdRuntimeError").d("reported runtime error", std::string(sb.GetString())));
}

#ifdef ENABLE_RTCSC
void GUIClient::renderCamera(
    const std::string& payload,
    smartScreenSDKInterfaces::AudioState microphoneAudioState,
    smartScreenSDKInterfaces::ConcurrentTwoWayTalk concurrentTwoWayTalk) {
    std::string liveViewControllerOptions;
    if (m_liveViewControllerOptionsConfig) {
        liveViewControllerOptions = m_liveViewControllerOptionsConfig.serialize();
    }
    auto message = messages::RenderCameraMessage(payload, liveViewControllerOptions);
    sendMessage(message);
}

void GUIClient::onCameraStateChanged(smartScreenSDKInterfaces::CameraState cameraState) {
    auto cameraStateStr = smartScreenSDKInterfaces::cameraStateToString(cameraState);
    auto message = messages::CameraStateChangedMessage(cameraStateStr);
    sendMessage(message);

    if (m_aplLiveViewExtension) {
        m_executor.submit([this, cameraStateStr]() { m_aplLiveViewExtension->setCameraState(cameraStateStr); });
    }
}

void GUIClient::onFirstFrameRendered() {
    m_executor.submit([this]() { executeCameraFirstFrameRendered(); });
}

void GUIClient::clearCamera() {
    auto message = messages::ClearCameraMessage();
    sendMessage(message);

    if (m_aplLiveViewExtension) {
        m_executor.submit([this]() {
            m_aplLiveViewExtension->onCameraCleared();
            m_aplClientBridge->clearDocument(LIVE_VIEW_UI_WINDOW_ID);
        });
    }
}
#endif

}  // namespace gui
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
