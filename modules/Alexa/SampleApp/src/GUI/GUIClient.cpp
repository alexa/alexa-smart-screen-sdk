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
#include <RegistrationManager/CustomerDataManager.h>
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

/// The message type for Hold To Talk.
static const std::string MESSAGE_TYPE_HOLD_TO_TALK("holdToTalk");

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
static const std::string MESSAGE_TYPE_DISPLAY_METRICS("displayMetrics");

/// The message type for toggling captions.
static const std::string MESSAGE_TYPE_TOGGLE_CAPTIONS("toggleCaptions");

/// The message type for toggling DoNotDisturb.
static const std::string MESSAGE_TYPE_TOGGLE_DONOTDISTURB("toggleDoNotDisturb");

/// Key for isSupported.
static const std::string IS_SUPPORTED_TAG("isSupported");

/// Key for APL max version.
static const std::string APL_MAX_VERSION_TAG("APLMaxVersion");

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

/// The window json key in the message.
static const std::string WINDOW_ID_TAG("windowId");

/// The result json key in the message.
static const std::string RESULT_TAG("result");

/// The error json key in the message.
static const std::string ERROR_TAG("error");

/// The event json key in the message.
static const std::string EVENT_TAG("event");

/// The drop frame count json key in the message.
static const std::string DROP_FRAME_COUNT_TAG("dropFrameCount");

/// Interface name to use for focus requests.
static const std::string APL_INTERFACE("Alexa.Presentation.APL");

/// Storage component name.
static const std::string COMPONENT_NAME{"GUIClient"};

/// Storage table name.
static const std::string TABLE_NAME{"GUIClient"};

/// Storage key name for APLMaxVersion entry.
static const std::string APL_MAX_VERSION_DB_KEY{"APLMaxVersion"};

/// Initial APL version to use at the first run and before any  GUI client is connected.
static const std::string INITIAL_APL_MAX_VERSION{"1.4"};

/// The key in our config file to find the root of GUI configuration
static const std::string GUI_CONFIGURATION_ROOT_KEY = "gui";

/// The key in our config file to find the root of VisualCharacteristics configuration
static const std::string VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY = "visualCharacteristics";

/// The key in our config file to find the root of app configuration
static const std::string APPCONFIG_CONFIGURATION_ROOT_KEY = "appConfig";

/// One second Autorelease timeout
static const std::chrono::seconds AUTORELEASE_DURATION{1};

namespace alexaSmartScreenSDK {
namespace sampleApp {
namespace gui {

using namespace alexaClientSDK::avsCommon::utils::json;
using namespace alexaClientSDK::avsCommon::utils::timing;
using namespace alexaClientSDK::avsCommon::sdkInterfaces::storage;

using namespace smartScreenSDKInterfaces;
using namespace smartScreenCapabilityAgents::alexaPresentation;
using namespace smartScreenCapabilityAgents::templateRuntime;

/**
 * Save APLMaxVersion persistently.
 *
 * @param miscStorage - Storage interface which used to save.
 * @param APLMaxVersion - The value to save.
 * @return @c true for success. @c false otherwise.
 */
static bool saveAPLMaxVersionInStorage(
    const std::shared_ptr<MiscStorageInterface>& miscStorage,
    const std::string& APLMaxVersion) {
    if (!miscStorage->put(COMPONENT_NAME, TABLE_NAME, APL_MAX_VERSION_DB_KEY, APLMaxVersion)) {
        ACSDK_ERROR(LX("saveAPLMaxVersionInStorage").m("Could not set new value"));
        return false;
    }

    ACSDK_DEBUG1(LX("saveAPLMaxVersionInStorage").m("succeeded"));

    return true;
}

/**
 * Open and initialize the storage interface.
 * @param miscStorage The storage handle.
 * @return @c true for success. @c false otherwise.
 */
static bool openStorage(const std::shared_ptr<MiscStorageInterface>& miscStorage) {
    if (!miscStorage->isOpened() && !miscStorage->open()) {
        ACSDK_DEBUG3(LX("openStorage").m("Couldn't open misc database. Creating."));

        if (!miscStorage->createDatabase()) {
            ACSDK_ERROR(LX("openStorageFailed").m("Could not create misc database."));
            return false;
        }
    }

    bool guiClientTableExists = false;
    if (!miscStorage->tableExists(COMPONENT_NAME, TABLE_NAME, &guiClientTableExists)) {
        ACSDK_ERROR(LX("openStorageFailed").m("Could not get table information misc database."));
        return false;
    }

    if (!guiClientTableExists) {
        ACSDK_DEBUG3(LX("openStorage").d("table doesn't exist", TABLE_NAME));
        if (!miscStorage->createTable(
                COMPONENT_NAME,
                TABLE_NAME,
                MiscStorageInterface::KeyType::STRING_KEY,
                MiscStorageInterface::ValueType::STRING_VALUE)) {
            ACSDK_ERROR(LX("openStorageFailed")
                            .d("reason", "Could not create table")
                            .d("table", TABLE_NAME)
                            .d("component", COMPONENT_NAME));
            return false;
        }
        if (!saveAPLMaxVersionInStorage(miscStorage, INITIAL_APL_MAX_VERSION)) {
            return false;
        }
    }

    return true;
}

static std::string getAPLMaxVersionFromStorage(const std::shared_ptr<MiscStorageInterface>& miscStorage) {
    std::string APLMaxVersion;
    if (!miscStorage->get(COMPONENT_NAME, TABLE_NAME, APL_MAX_VERSION_DB_KEY, &APLMaxVersion)) {
        ACSDK_ERROR(LX("getAPLMaxVersionFromStorageFailed").d("reason", "storage failure"));
    }
    ACSDK_DEBUG3(LX(__func__).d("APLMaxVersion", APLMaxVersion));
    return APLMaxVersion;
}

std::shared_ptr<GUIClient> GUIClient::create(
    std::shared_ptr<MessagingServerInterface> serverImplementation,
    const std::shared_ptr<MiscStorageInterface>& miscStorage,
    const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager) {
    if (!serverImplementation) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullServerImplementation"));
        return nullptr;
    }
    if (!openStorage(miscStorage)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullMiscStorage"));
        return nullptr;
    }

    if (!customerDataManager) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullCustomerDataManager"));
        return nullptr;
    }
    std::string APLMaxVersion = getAPLMaxVersionFromStorage(miscStorage);
    if (APLMaxVersion.empty()) {
        ACSDK_ERROR(LX("createFailed").d("reason", "couldn't find saved APLMaxVersion"));
        return nullptr;
    }

    return std::shared_ptr<GUIClient>(
        new GUIClient(serverImplementation, miscStorage, APLMaxVersion, customerDataManager));
}

GUIClient::GUIClient(
    std::shared_ptr<MessagingServerInterface> serverImplementation,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::storage::MiscStorageInterface>& miscStorage,
    const std::string& APLMaxVersion,
    const std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> customerDataManager) :
        RequiresShutdown{"GUIClient"},
        CustomerDataHandler{customerDataManager},
        m_serverImplementation{serverImplementation},
        m_hasServerStarted{false},
        m_initMessageReceived{false},
        m_errorState{false},
        m_APLMaxVersion{APLMaxVersion},
        m_shouldRestart{false},
        m_miscStorage{miscStorage},
        m_captionManager{SmartScreenCaptionStateManager(miscStorage)} {
    m_messageHandlers.emplace(
        MESSAGE_TYPE_TAP_TO_TALK, [this](rapidjson::Document& payload) { executeHandleTapToTalk(payload); });
    m_messageHandlers.emplace(
        MESSAGE_TYPE_HOLD_TO_TALK, [this](rapidjson::Document& payload) { executeHandleHoldToTalk(payload); });
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
    m_messageHandlers.emplace(MESSAGE_TYPE_TOGGLE_DONOTDISTURB, [this](rapidjson::Document& payload) {
        m_guiManager->handleToggleDoNotDisturbEvent();
    });
}

void GUIClient::doShutdown() {
    ACSDK_DEBUG3(LX(__func__));
    stop();
    m_executor.shutdown();
    m_guiManager.reset();
    m_aplClientBridge.reset();
    m_messageListener.reset();
    m_observer.reset();
    m_miscStorage.reset();
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

void GUIClient::setAplClientBridge(std::shared_ptr<AplClientBridge> aplClientBridge) {
    ACSDK_DEBUG3(LX(__func__));
    m_executor.submit([this, aplClientBridge]() { m_aplClientBridge = aplClientBridge; });
}

bool GUIClient::acquireFocus(
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    ACSDK_DEBUG5(LX(__func__));

    return m_executor
        .submit([this, channelName, channelObserver]() { return executeAcquireFocus(channelName, channelObserver); })
        .get();
}

bool GUIClient::releaseFocus(
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    ACSDK_DEBUG5(LX(__func__));
    return m_executor
        .submit([this, channelName, channelObserver]() { return executeReleaseFocus(channelName, channelObserver); })
        .get();
}

bool GUIClient::executeAcquireFocus(
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_guiManager->handleFocusAcquireRequest(channelName, channelObserver);
}

bool GUIClient::executeReleaseFocus(
    std::string channelName,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> channelObserver) {
    return m_guiManager->handleFocusReleaseRequest(channelName, channelObserver);
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
        m_serverThread.detach();
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

void GUIClient::provideState(const unsigned int stateRequestToken) {
    m_executor.submit([this, stateRequestToken]() { m_aplClientBridge->provideState(stateRequestToken); });
}

void GUIClient::interruptCommandSequence() {
    m_guiManager->onUserEvent();
    m_executor.submit([this]() { m_aplClientBridge->interruptCommandSequence(); });
}

void GUIClient::onPresentationSessionChanged() {
    m_executor.submit([this]() { m_aplClientBridge->onPresentationSessionChanged(); });
}

void GUIClient::executeHandleTapToTalk(rapidjson::Document& message) {
    m_guiManager->handleTapToTalk();
}

void GUIClient::executeHandleHoldToTalk(rapidjson::Document& message) {
    m_guiManager->handleHoldToTalk();
}

void GUIClient::executeHandleFocusAcquireRequest(rapidjson::Document& message) {
    ACSDK_CRITICAL(LX("handleFocusAcquireRequest"));
    APLToken token;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleFocusAcquireRequestFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string channelName;
    if (!jsonUtils::retrieveValue(message, "channelName", &channelName)) {
        ACSDK_ERROR(LX("handleFocusAcquireRequestFailed").d("reason", "channelNameNotFound"));
        return;
    }

    executeProcessFocusAcquireRequest(token, channelName, APL_INTERFACE);
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

void GUIClient::executeProcessFocusAcquireRequest(
    const APLToken token,
    const std::string& channelName,
    const std::string& avsInterface) {
    executeFocusAcquireRequest(token, channelName, avsInterface);
}

void GUIClient::executeFocusAcquireRequest(
    const APLToken token,
    const std::string& channelName,
    const std::string& avsInterface) {
    bool result = true;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelObserverInterface> focusObserver;
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        if (m_focusObservers.count(token) == 0) {
            m_focusObservers[token] = std::make_shared<ProxyFocusObserver>(token, shared_from_this(), channelName);
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

    result = executeAcquireFocus(channelName, focusObserver);
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusAcquireRequestFail").d("token", token).d("reason", "acquireChannel returned false"));
        executeSendFocusResponse(token, false);
        return;
    }

    executeSendFocusResponse(token, true);
}

void GUIClient::executeHandleFocusReleaseRequest(rapidjson::Document& message) {
    APLToken token;
    if (!jsonUtils::retrieveValue(message, TOKEN_TAG, &token)) {
        ACSDK_ERROR(LX("handleFocusReleaseRequestFailed").d("reason", "tokenNotFound"));
        return;
    }

    std::string channelName;
    if (!jsonUtils::retrieveValue(message, "channelName", &channelName)) {
        ACSDK_ERROR(LX("handleFocusReleaseRequestFailed").d("reason", "channelNameNotFound"));
        return;
    }

    executeFocusReleaseRequest(token, channelName);
}

void GUIClient::executeFocusReleaseRequest(const APLToken token, const std::string& channelName) {
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

    result = executeReleaseFocus(channelName, focusObserver);
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusReleaseRequestFail").d("token", token).d("reason", "releaseChannel returned false"));
        executeSendFocusResponse(token, false);
        return;
    }
    executeSendFocusResponse(token, true);
}

void GUIClient::executeSendFocusResponse(const APLToken token, const bool result) {
    auto message = messages::FocusResponseMessage(token, result);
    sendMessage(message);
}

void GUIClient::executeHandleOnFocusChangedReceivedConfirmation(rapidjson::Document& message) {
    APLToken token;
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

    m_aplClientBridge->renderDocument(payload, AlexaPresentation::getNonAPLDocumentToken(), windowId);
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

void GUIClient::executeHandleActivityEvent(rapidjson::Document& message) {
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

    m_aplClientBridge->onMessage(payload);
}

void GUIClient::executeHandleDeviceWindowState(rapidjson::Document& message) {
    std::string payload;
    if (!jsonUtils::retrieveValue(message, PAYLOAD_TAG, &payload)) {
        ACSDK_ERROR(LX("handleDeviceWindowStateFailed").d("reason", "payloadNotFound"));
        return;
    }

    m_guiManager->handleDeviceWindowState(payload);
}

void GUIClient::executeHandleRenderComplete(rapidjson::Document& message) {
    m_guiManager->handleRenderComplete();
}

void GUIClient::executeHandleDisplayMetrics(rapidjson::Document& message) {
    if (!message.HasMember(PAYLOAD_TAG)) {
        ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "payloadNotFound"));
        return;
    }

    const auto& payload = message[PAYLOAD_TAG];

    if (payload.IsArray()) {
        std::vector<APLClient::DisplayMetric> metrics;
        for (rapidjson::SizeType i = 0; i < payload.Size(); i++) {
            const auto& jsonMetric = payload[i];
            APLClient::DisplayMetric metric;
            std::string kind;

            if (!jsonUtils::retrieveValue(jsonMetric, "kind", &kind)) {
                ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "missingMetricKind"));
                return;
            }

            if (kind == "timer") {
                metric.kind = APLClient::DisplayMetricKind::kTimer;
            } else if (kind == "counter") {
                metric.kind = APLClient::DisplayMetricKind::kCounter;
            } else {
                ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "unsupportedMetricKind"));
                return;
            }

            if (!jsonUtils::retrieveValue(jsonMetric, "name", &metric.name)) {
                ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "missingMetricName"));
                return;
            }

            rapidjson::Value::ConstMemberIterator iterator;
            if (!jsonUtils::findNode(jsonMetric, "value", &iterator)) {
                ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "missingMetricValue"));
                return;
            }

            double d_value = 0;
            uint64_t i_value = 0;
            if (iterator->value.IsDouble() && jsonUtils::retrieveValue(jsonMetric, "value", &d_value)) {
                metric.value = static_cast<uint64_t>(d_value);
            } else if (iterator->value.IsUint64() && jsonUtils::retrieveValue(jsonMetric, "value", &i_value)) {
                metric.value = static_cast<uint64_t>(i_value);
            } else {
                ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "incorrectTypeOrValue"));
                return;
            }

            metrics.emplace_back(metric);
        }
        m_guiManager->handleDisplayMetrics(metrics);
    } else {
        // Deprecated, retained for backward compatibility only while the viewhost is updated
        uint64_t dropFrameCount;
        if (!jsonUtils::retrieveValue(payload, DROP_FRAME_COUNT_TAG, &dropFrameCount)) {
            ACSDK_ERROR(LX("executeHandleDisplayMetrics").d("reason", "dropFrameCountNotFound"));
            return;
        }

        m_guiManager->handleDisplayMetrics(dropFrameCount);
    }
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
    const std::string& jsonPayload,
    alexaClientSDK::avsCommon::avs::FocusState focusState) {
    auto message = messages::RenderTemplateMessage(jsonPayload);
    sendMessage(message);
}

void GUIClient::clearTemplateCard() {
    ACSDK_DEBUG5(LX("clearTemplateCard"));
    m_executor.submit([this]() {
        m_aplClientBridge->clearDocument();

        auto message = messages::ClearRenderTemplateCardMessage();
        executeSendMessage(message);
    });
}

void GUIClient::renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId) {
    m_executor.submit(
        [this, jsonPayload, token, windowId]() { m_aplClientBridge->renderDocument(jsonPayload, token, windowId); });
}

void GUIClient::clearDocument() {
    ACSDK_DEBUG5(LX("clearDocument"));
    m_executor.submit([this]() {
        m_aplClientBridge->clearDocument();

        auto message = messages::ClearDocumentMessage();
        executeSendMessage(message);
    });
}

void GUIClient::clearData() {
    ACSDK_DEBUG5(LX(__func__));
    if (!m_miscStorage->clearTable(COMPONENT_NAME, TABLE_NAME)) {
        ACSDK_ERROR(LX("clearTableFailed").d("reason", "unable to clear the table from the database"));
    }
    if (!m_miscStorage->deleteTable(COMPONENT_NAME, TABLE_NAME)) {
        ACSDK_ERROR(LX("deleteTableFailed").d("reason", "unable to delete the table from the database"));
    }
}

void GUIClient::renderPlayerInfoCard(
    const std::string& jsonPayload,
    smartScreenSDKInterfaces::AudioPlayerInfo info,
    alexaClientSDK::avsCommon::avs::FocusState focusState,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MediaPropertiesInterface> mediaProperties) {
    auto message = messages::RenderPlayerInfoMessage(jsonPayload, info);
    sendMessage(message);
}

void GUIClient::clearPlayerInfoCard() {
    ACSDK_DEBUG5(LX("clearPlayerInfoCard"));

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

std::string GUIClient::getMaxAPLVersion() {
    return m_APLMaxVersion;
}

void GUIClient::onLogout() {
    m_shouldRestart = true;
    m_cond.notify_all();
    clearData();
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

void GUIClient::executeSendGuiConfiguration() {
    ACSDK_DEBUG9(LX(__func__));

    /// Get the root ConfigurationNode.
    auto configurationRoot = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();

    /// Get the root of GUI ConfigurationNode.
    auto configurationGui = configurationRoot[GUI_CONFIGURATION_ROOT_KEY];

    /// Get the ConfigurationNode contains visualCharacteristics config array.
    auto visualCharacteristics = configurationGui.getArray(VISUALCHARACTERISTICS_CONFIGURATION_ROOT_KEY);

    // Get the ConfigurationNode contains appConfig.
    auto appConfig = configurationGui[APPCONFIG_CONFIGURATION_ROOT_KEY];

    auto appConfigString = appConfig.serialize();
    auto visualCharacteristicsString = visualCharacteristics.serialize();

#ifndef _MSC_VER
    auto message = messages::GuiConfigurationMessage(visualCharacteristicsString, appConfigString);
    sendMessage(message);
#else
    auto payloadWithHeader = R"({"type": "guiConfiguration", "payload": {"visualCharacteristics": )" +
                             visualCharacteristicsString + R"(, "appConfig": )" + appConfigString + R"(}})";

    writeMessage(payloadWithHeader);
#endif
}

bool GUIClient::executeProcessInitResponse(const rapidjson::Document& message) {
    bool isSupported;
    if (!jsonUtils::retrieveValue(message, IS_SUPPORTED_TAG, &isSupported)) {
        ACSDK_ERROR(LX("processInitResponseFailed").d("reason", "isSupportedNotFound"));
        m_errorState = true;

        return false;
    }

    std::string newAPLMaxVersion;
    if (!jsonUtils::retrieveValue(message, APL_MAX_VERSION_TAG, &newAPLMaxVersion)) {
        ACSDK_ERROR(LX("processInitResponseFailed").d("reason", "APLVersionNotFound"));
        m_errorState = true;

        return false;
    }

    if (!isSupported) {
        ACSDK_ERROR(LX("processInitResponseFailed")
                        .d("reason", "Not Supported SDK")
                        .d("SDKVersion", alexaClientSDK::avsCommon::utils::sdkVersion::getCurrentVersion())
                        .d("APL Version", m_APLMaxVersion));

        // Don't get into error state, so GUI client can connect with supported version.
        return false;
    }

    m_initMessageReceived = true;
    if (newAPLMaxVersion != m_APLMaxVersion) {
        ACSDK_DEBUG1(
            LX("executeProcessInitResponse").d("old maxAPL", m_APLMaxVersion).d("new max APL", newAPLMaxVersion));
        saveAPLMaxVersionInStorage(m_miscStorage, newAPLMaxVersion);

        // Restart in order to call Capabilities API with the new APLMAxVersion
        m_shouldRestart = true;
    }

    ACSDK_INFO(LX("executeProcessInitResponse").d("APL Max Version", m_APLMaxVersion));
    m_cond.notify_all();
    if (m_initThread.joinable()) {
        m_initThread.join();
    }
    executeSendGuiConfiguration();
    return true;
}

void GUIClient::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
    m_limitedInteraction = m_limitedInteraction || (newState == AuthObserverInterface::State::UNRECOVERABLE_ERROR);
}

void GUIClient::onCapabilitiesStateChange(
    CapabilitiesObserverInterface::State newState,
    CapabilitiesObserverInterface::Error newError,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& addedOrUpdatedEndpoints,
    const std::vector<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier>& deletedEndpoints) {
    m_limitedInteraction = m_limitedInteraction || (newState == CapabilitiesObserverInterface::State::FATAL_ERROR);
}

GUIClient::ProxyFocusObserver::ProxyFocusObserver(
    const APLToken token,
    std::shared_ptr<GUIClient> guiClient,
    const std::string& channelName) :
        m_token{token}, m_focusBridge{guiClient}, m_channelName{channelName} {
}

void GUIClient::ProxyFocusObserver::onFocusChanged(
    alexaClientSDK::avsCommon::avs::FocusState newFocus,
    alexaClientSDK::avsCommon::avs::MixingBehavior behavior) {
    if (newFocus != alexaClientSDK::avsCommon::avs::FocusState::NONE) {
        m_focusBridge->startAutoreleaseTimer(m_token, m_channelName);
    }
    m_focusBridge->sendOnFocusChanged(m_token, newFocus);
}

void GUIClient::startAutoreleaseTimer(const APLToken token, const std::string& channelName) {
    std::shared_ptr<alexaClientSDK::avsCommon::utils::timing::Timer> timer =
        std::make_shared<alexaClientSDK::avsCommon::utils::timing::Timer>();
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        m_autoReleaseTimers[token] = timer;
    }

    timer->start(AUTORELEASE_DURATION, [this, token, channelName] { autoRelease(token, channelName); });
}

void GUIClient::autoRelease(const APLToken token, const std::string& channelName) {
    ACSDK_DEBUG5(LX("autoRelease").d("token", token).d("channelName", channelName));
    m_executor.submit([this, token, channelName]() {
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
        m_guiManager->handleFocusReleaseRequest(channelName, focusObserver);
    });
}

void GUIClient::sendOnFocusChanged(const APLToken token, const alexaClientSDK::avsCommon::avs::FocusState state) {
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

}  // namespace gui
}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
