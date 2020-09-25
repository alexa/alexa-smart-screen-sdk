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

#include "APLClientSandbox/Logger.h"
#include "APLClientSandbox/WebSocketSDKLogger.h"

void WebSocketSDKLogger::write(websocketpp::log::level channel, std::string const& msg) {
    write(channel, msg.c_str());
}

void WebSocketSDKLogger::write(websocketpp::log::level channel, char const* msg) {
    if (m_channelTypeHint == websocketpp::log::channel_type_hint::access) {
        logAccessMessage(channel, msg);
    } else {
        logErrorMessage(channel, msg);
    }
}

void WebSocketSDKLogger::logErrorMessage(websocketpp::log::level channel, char const* msg) const {
    switch (channel) {
        case websocketpp::log::elevel::devel:
        case websocketpp::log::elevel::library:
            Logger::debug("WebSocketSDKLogger::ErrorLog", msg);
            return;
        case websocketpp::log::elevel::info:
            Logger::info("WebSocketSDKLogger::ErrorLog", msg);
            return;
        case websocketpp::log::elevel::warn:
            Logger::warn("WebSocketSDKLogger::ErrorLog", msg);
            return;
        case websocketpp::log::elevel::rerror:
            Logger::error("WebSocketSDKLogger::ErrorLog", msg);
            return;
        case websocketpp::log::elevel::fatal:
            Logger::error("WebSocketSDKLogger::ErrorLog", msg);
            return;
        default:
            Logger::info("WebSocketSDKLogger::ErrorLog", msg);
            return;
    }
}

void WebSocketSDKLogger::logAccessMessage(websocketpp::log::level, char const* msg) const {
    // Very verbose, uncomment if required
    // Logger::debug("WebSocketSDKLogger::AccessLog", msg);
}