/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/FocusBridge.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::utils::timing;

using namespace smartScreenSDKInterfaces;
/// String to identify log entries originating from this file.
static const std::string TAG("FocusBridge");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// One second Autorelease timeout
static const std::chrono::seconds AUTORELEASE_DURATION{1};

FocusBridge::FocusBridge(
    std::shared_ptr<smartScreenClient::SmartScreenClient> client,
    std::shared_ptr<MessagingInterface> messagingInterface) :
        RequiresShutdown{"FocusBridge"},
        m_client{client},
        m_messagingInterface{messagingInterface} {};

void FocusBridge::processFocusAcquireRequest(
    const APLToken token,
    const std::string& channelName,
    const std::string& avsInterface) {
    m_executor.submit(
        [this, token, channelName, avsInterface]() { executeFocusAcquireRequest(token, channelName, avsInterface); });
}

void FocusBridge::processFocusReleaseRequest(const APLToken token, const std::string& channelName) {
    m_executor.submit([this, token, channelName]() { executeFocusReleaseRequest(token, channelName); });
}

void FocusBridge::executeFocusAcquireRequest(
    const APLToken token,
    const std::string& channelName,
    const std::string& avsInterface) {
    bool result = true;
    auto focusManagerInterface = getFocusManagerInterfaceForChannel(channelName);

    std::shared_ptr<ChannelObserverInterface> focusObserver;
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
        sendFocusResponse(token, false);
        return;
    }

    result = focusManagerInterface->acquireChannel(channelName, focusObserver, avsInterface);
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusAcquireRequestFail").d("token", token).d("reason", "acquireChannel returned false"));
        sendFocusResponse(token, false);
        return;
    }

    sendFocusResponse(token, true);
}

void FocusBridge::executeFocusReleaseRequest(const APLToken token, const std::string& channelName) {
    bool result = true;
    auto focusManagerInterface = getFocusManagerInterfaceForChannel(channelName);

    std::shared_ptr<ChannelObserverInterface> focusObserver;
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
        sendFocusResponse(token, false);
        return;
    }

    result = focusManagerInterface->releaseChannel(channelName, focusObserver).get();
    if (!result) {
        ACSDK_ERROR(
            LX("executeFocusReleaseRequestFail").d("token", token).d("reason", "releaseChannel returned false"));
        sendFocusResponse(token, false);
        return;
    }
    sendFocusResponse(token, true);
}

void FocusBridge::sendFocusResponse(const APLToken token, const bool result) {
    auto payloadWithHeader = R"({"type": "focusResponse", "token": )" + std::to_string(token) + R"(, "result":")" +
                             (result ? "true" : "false") + R"("})";
    m_messagingInterface->writeMessage(payloadWithHeader);
}

void FocusBridge::sendOnFocusChanged(const APLToken token, const FocusState state) {
    auto payloadWithHeader = R"({"type": "onFocusChanged", "token": )" + std::to_string(token) +
                             R"(, "channelState":")" + focusStateToString(state) + R"("})";
    m_messagingInterface->writeMessage(payloadWithHeader);

    if (state == FocusState::NONE) {
        // Remove observer and timer when released.
        std::lock_guard<std::mutex> lock{m_mapMutex};
        if (m_focusObservers.erase(token) == 0) {
            ACSDK_WARN(LX("tokenNotFoundWhenRemovingObserver").d("token", token));
        }
        if (m_autoReleaseTimers.erase(token) == 0) {
            ACSDK_WARN(LX("tokenNotFoundWhenRemovingAutoReleaseTimer").d("token", token));
        }
    }
}

void FocusBridge::processOnFocusChangedReceivedConfirmation(const APLToken token) {
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

void FocusBridge::autoRelease(const APLToken token, const std::string& channelName) {
    ACSDK_WARN(LX("autoRelease").d("token", token).d("channelName", channelName));
    auto focusManagerInterface = getFocusManagerInterfaceForChannel(channelName);
    std::shared_ptr<avsCommon::sdkInterfaces::ChannelObserverInterface> focusObserver;
    std::shared_ptr<avsCommon::utils::timing::Timer> autoReleaseTimer;
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        focusObserver = m_focusObservers[token];
        if (!focusObserver) {
            ACSDK_CRITICAL(LX("autoReleaseFailed").d("token", token).d("reason", "focusObserver is null"));
            return;
        }
    }
    m_executor.submit([focusManagerInterface, channelName, focusObserver] {
        focusManagerInterface->releaseChannel(channelName, focusObserver);
    });
}

/**
 * Starting timer to release channel in situations when focus operation result or
 * onFocusChanged event was not received by GUI so it will not know if it needs to release it.
 *
 * @param token The APL token.
 * @param channelName The channel to release.
 */
void FocusBridge::startAutoreleaseTimer(const APLToken token, const std::string& channelName) {
    std::shared_ptr<avsCommon::utils::timing::Timer> timer = std::make_shared<Timer>();
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        m_autoReleaseTimers[token] = timer;
    }

    timer->start(AUTORELEASE_DURATION, [this, token, channelName] { autoRelease(token, channelName); });
}

std::shared_ptr<avsCommon::sdkInterfaces::FocusManagerInterface> FocusBridge::getFocusManagerInterfaceForChannel(
    const std::string& channelName) {
    if (channelName == FocusManagerInterface::VISUAL_CHANNEL_NAME) {
        return m_client->getVisualFocusManager();
    } else {
        return m_client->getAudioFocusManager();
    }
}

void FocusBridge::doShutdown() {
    {
        std::lock_guard<std::mutex> lock{m_mapMutex};
        for (auto& item : m_autoReleaseTimers) {
            if (item.second) {
                item.second->stop();
                item.second.reset();
            }
        }
        m_autoReleaseTimers.clear();

        for (auto& item : m_focusObservers) {
            if (item.second) {
                item.second.reset();
            }
        }
        m_focusObservers.clear();
    }
    m_client.reset();
}

FocusBridge::ProxyFocusObserver::ProxyFocusObserver(
    const APLToken token,
    std::shared_ptr<FocusBridge> focusBridge,
    const std::string& channelName) :
        m_token{token},
        m_focusBridge{focusBridge},
        m_channelName{channelName} {
}

void FocusBridge::ProxyFocusObserver::onFocusChanged(FocusState newFocus) {
    if (newFocus != FocusState::NONE) {
        m_focusBridge->startAutoreleaseTimer(m_token, m_channelName);
    }
    m_focusBridge->sendOnFocusChanged(m_token, newFocus);
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
