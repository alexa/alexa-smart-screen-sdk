/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <iostream>
#include <sstream>

#include "SampleApp/JsonUIManager.h"

#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include "AVSCommon/Utils/SDKVersion.h"

#include "SampleApp/ConsolePrinter.h"

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

using namespace smartScreenSDKInterfaces;

static const std::string VERSION = avsCommon::utils::sdkVersion::getCurrentVersion();

JsonUIManager::JsonUIManager(
    std::shared_ptr<smartScreenSDKInterfaces::GUIClientInterface> guiClientInterface,
    std::shared_ptr<avsCommon::utils::DeviceInfo> deviceInfo) :
        m_dialogState{DialogUXState::IDLE},
        m_capabilitiesState{CapabilitiesObserverInterface::State::UNINITIALIZED},
        m_capabilitiesError{CapabilitiesObserverInterface::Error::UNINITIALIZED},
        m_authState{AuthObserverInterface::State::UNINITIALIZED},
        m_authCheckCounter{0},
        m_connectionStatus{avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED},
        m_guiClientInterface{guiClientInterface},
        m_deviceInfo{deviceInfo} {
}

void JsonUIManager::onDialogUXStateChanged(DialogUXState state) {
    m_executor.submit([this, state]() {
        if (state == m_dialogState) {
            return;
        }
        m_dialogState = state;
        reportAlexaState();
    });
}

void JsonUIManager::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    m_executor.submit([this, status]() {
        if (m_connectionStatus == status) {
            return;
        }
        m_connectionStatus = status;
        reportAlexaState();
    });
}

void JsonUIManager::onSettingChanged(const std::string& key, const std::string& value) {
    m_executor.submit([key, value]() {
        std::string msg = key + " set to " + value;
        ConsolePrinter::prettyPrint(msg);
    });
}

void JsonUIManager::onSpeakerSettingsChanged(
    const SpeakerManagerObserverInterface::Source& source,
    const avsCommon::sdkInterfaces::ChannelVolumeInterface::Type& type,
    const SpeakerInterface::SpeakerSettings& settings) {
    m_executor.submit([source, type, settings]() {
        std::ostringstream oss;
        oss << "SOURCE:" << source << " TYPE:" << type << " VOLUME:" << static_cast<int>(settings.volume)
            << " MUTE:" << settings.mute;
        ConsolePrinter::prettyPrint(oss.str());
    });
}

void JsonUIManager::onSetIndicator(avsCommon::avs::IndicatorState state) {
    m_executor.submit([state]() {
        std::ostringstream oss;
        oss << "NOTIFICATION INDICATOR STATE: " << state;
        ConsolePrinter::prettyPrint(oss.str());
    });
}

void JsonUIManager::onRequestAuthorization(const std::string& url, const std::string& code) {
    m_executor.submit([this, url, code]() {
        m_authCheckCounter = 0;
        ConsolePrinter::prettyPrint("NOT YET AUTHORIZED");
        std::ostringstream oss;
        oss << "To authorize, browse to: '" << url << "' and enter the code: " << code;
        ConsolePrinter::prettyPrint(oss.str());
    });

    m_authUrl = url;
    m_authCode = code;

    sendAuthorizationRequestMessage();
}

void JsonUIManager::onCheckingForAuthorization() {
    m_executor.submit([this]() {
        std::ostringstream oss;
        oss << "Checking for authorization (" << ++m_authCheckCounter << ")...";
        ConsolePrinter::prettyPrint(oss.str());
    });

    sendAuthorizationRequestMessage();
}
void JsonUIManager::sendAuthorizationRequestMessage() {
    auto message = messages::AuthorizationRequestMessage(m_authUrl, m_authCode, m_deviceInfo->getClientId());
    m_guiClientInterface->sendMessage(message);
}

void JsonUIManager::onAuthStateChange(AuthObserverInterface::State newState, AuthObserverInterface::Error newError) {
    if (m_authState != newState) {
        m_authState = newState;

        m_executor.submit([this, newError]() {
            switch (m_authState) {
                case AuthObserverInterface::State::UNINITIALIZED:
                    break;
                case AuthObserverInterface::State::REFRESHED:
                    ConsolePrinter::prettyPrint("Authorized!");
                    break;
                case AuthObserverInterface::State::EXPIRED:
                    ConsolePrinter::prettyPrint("AUTHORIZATION EXPIRED");
                    break;
                case AuthObserverInterface::State::UNRECOVERABLE_ERROR:
                    std::ostringstream oss;
                    oss << "UNRECOVERABLE AUTHORIZATION ERROR: " << newError;
                    break;
            }
        });

        std::string authState = "UNINITIALIZED";
        switch (m_authState) {
            case AuthObserverInterface::State::UNINITIALIZED:
                authState = "UNINITIALIZED";
                break;
            case AuthObserverInterface::State::REFRESHED:
                authState = "REFRESHED";
                break;
            case AuthObserverInterface::State::EXPIRED:
                authState = "EXPIRED";
                break;
            case AuthObserverInterface::State::UNRECOVERABLE_ERROR:
                authState = "ERROR";
                break;
        }

        auto message = messages::AuthorizationChangedMessage(authState);
        m_guiClientInterface->sendMessage(message);
    }
}

void JsonUIManager::onCapabilitiesStateChange(
    CapabilitiesObserverInterface::State newState,
    CapabilitiesObserverInterface::Error newError) {
    m_executor.submit([this, newState, newError]() {
        if ((m_capabilitiesState != newState) && (m_capabilitiesError != newError)) {
            m_capabilitiesState = newState;
            m_capabilitiesError = newError;
            if (CapabilitiesObserverInterface::State::FATAL_ERROR == m_capabilitiesState) {
                std::ostringstream oss;
                oss << "UNRECOVERABLE CAPABILITIES API ERROR: " << m_capabilitiesError;
            }
        }
    });
}

void JsonUIManager::printWelcomeScreen() {
}

void JsonUIManager::printHelpScreen() {
}

void JsonUIManager::printLimitedHelp() {
}

void JsonUIManager::printSettingsScreen() {
}

void JsonUIManager::printLocaleScreen() {
}

void JsonUIManager::printSpeakerControlScreen() {
}

void JsonUIManager::printFirmwareVersionControlScreen() {
}

void JsonUIManager::printVolumeControlScreen() {
}

void JsonUIManager::printESPControlScreen(
    bool support,
    const std::string& voiceEnergy,
    const std::string& ambientEnergy) {
    m_executor.submit([support, voiceEnergy, ambientEnergy]() {
        std::string screen = "";
        screen += "|\n";
        screen += "| support       = ";
        screen += support ? "true\n" : "false\n";
        screen += "| voiceEnergy   = " + voiceEnergy + "\n";
        screen += "| ambientEnergy = " + ambientEnergy + "\n";
        screen += "+----------------------------------------------------------------------------+\n";
        ConsolePrinter::simplePrint(screen);
    });
}

void JsonUIManager::printCommsControlScreen() {
}

void JsonUIManager::printErrorScreen() {
    m_executor.submit([]() { ConsolePrinter::prettyPrint("Invalid Option"); });
}

void JsonUIManager::microphoneOff() {
    m_executor.submit([]() { ConsolePrinter::prettyPrint("Microphone Off!"); });
}

void JsonUIManager::printResetConfirmation() {
}

void JsonUIManager::printReauthorizeConfirmation() {
}

void JsonUIManager::printResetWarning() {
}

void JsonUIManager::microphoneOn() {
    m_executor.submit([this]() { reportAlexaState(); });
}

void JsonUIManager::reportAlexaState() {
    std::string alexaState;

    switch (m_connectionStatus) {
        case alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED:
            alexaState = "DISCONNECTED";
            break;
        case alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING:
            alexaState = "CONNECTING";
            break;
        case alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED:
            switch (m_dialogState) {
                /*
                 * This is an intermediate state after a SPEAK directive is completed. In the case of a speech burst the
                 * next SPEAK could kick in or if its the last SPEAK directive ALEXA moves to the IDLE state. So we do
                 * nothing for this state.
                 */
                case alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::DialogUXState::FINISHED:
                    break;
                default:
                    alexaState =
                        alexaClientSDK::avsCommon::sdkInterfaces::DialogUXStateObserverInterface::stateToString(
                            m_dialogState);
                    break;
            }
            break;
    }

    auto message = messages::AlexaStateChangedMessage(alexaState);
    m_guiClientInterface->sendMessage(message);

    m_executor.submit([alexaState]() {
        std::ostringstream oss;
        oss << "ALEXA STATE: " << alexaState;
        ConsolePrinter::prettyPrint(oss.str());
    });
}

void JsonUIManager::onConnectionOpened() {
    m_executor.submit([]() { ConsolePrinter::prettyPrint("Message Server Connection Opened."); });
    m_executor.submit([this]() { reportAlexaState(); });
}

void JsonUIManager::onConnectionClosed() {
    m_executor.submit([]() { ConsolePrinter::prettyPrint("Message Server Connection Closed."); });
    m_executor.submit([this]() { reportAlexaState(); });
}

void JsonUIManager::printESPDataOverrideNotSupported() {
    m_executor.submit([]() { ConsolePrinter::simplePrint("Cannot override ESP Value in this device."); });
}

void JsonUIManager::printESPNotSupported() {
    m_executor.submit([]() { ConsolePrinter::simplePrint("ESP is not supported in this device."); });
}

void JsonUIManager::printCommsNotSupported() {
    m_executor.submit([]() { ConsolePrinter::simplePrint("Comms is not supported in this device."); });
}

void JsonUIManager::setFailureStatus(const std::string& status) {
    if (!status.empty() && status != m_failureStatus) {
        m_failureStatus = status;
        printLimitedHelp();
    }
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK
