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

#include <iostream>
#include <memory>

#include "APLClientSandbox/Logger.h"
#include "APLClientSandbox/WebSocketServer.h"

using namespace websocketpp::lib::placeholders;
using server = websocketpp::server<WebSocketConfig>;
using connection_hdl = websocketpp::connection_hdl;

WebSocketServer::WebSocketServer(const std::string& interface, const unsigned short port) {
    websocketpp::lib::error_code errorCode;
    m_webSocketServer.init_asio(errorCode);
    if (errorCode) {
        logError("server::init_asio", errorCode);
        return;
    }

    m_webSocketServer.set_reuse_addr(true);
    m_webSocketServer.set_open_handler(bind(&WebSocketServer::onConnectionOpen, this, _1));
    m_webSocketServer.set_close_handler(bind(&WebSocketServer::onConnectionClose, this, _1));
    m_webSocketServer.set_message_handler(bind(&WebSocketServer::onMessage, this, _1, _2));

#ifdef ENABLE_WEBSOCKET_SSL
    m_webSocketServer.set_tls_init_handler(bind(&WebSocketServer::onTlsInit, this));
#endif

    m_webSocketServer.set_validate_handler(bind(&WebSocketServer::onValidate, this, _1));

    auto endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(interface), port);
    m_webSocketServer.listen(endpoint, errorCode);

    if (errorCode) {
        logError("server::listen", errorCode);
        return;
    }

    m_initialised = true;
}

void WebSocketServer::setMessageListener(std::shared_ptr<MessageListenerInterface> messageListener) {
    m_messageListener = messageListener;
}

bool WebSocketServer::start() {
    if (!m_initialised) {
        logError("startFailed", "server not initialised");
        return false;
    }

    websocketpp::lib::error_code errorCode;

    m_webSocketServer.start_accept(errorCode);
    if (errorCode) {
        logError("server::start_accept", errorCode);
        return false;
    }

    auto endpoint = m_webSocketServer.get_local_endpoint(errorCode);
    if (errorCode) {
        logError("server::get_local_endpoint", errorCode);
        return false;
    }

    Logger::info(
        "WebsocketServer::start",
        "Listening for websocket connections. interface:",
        endpoint.address(),
        "port:",
        endpoint.port());

    m_webSocketServer.run();

    return true;
}

void WebSocketServer::stop() {
    websocketpp::lib::error_code errorCode;
    m_webSocketServer.stop_listening(errorCode);
    if (errorCode) {
        logError("server::stop_listening", errorCode);
    }

    m_webSocketServer.close(m_connection, websocketpp::close::status::going_away, "shutting down", errorCode);
    if (errorCode) {
        logError("server::close", errorCode);
    }

    m_connection.reset();
}

void WebSocketServer::writeMessage(const std::string& payload) {
    websocketpp::lib::error_code errorCode;

    m_webSocketServer.send(m_connection, payload, websocketpp::frame::opcode::text, errorCode);
    if (errorCode) {
        logError("server::send", errorCode);
    }
}

void WebSocketServer::onConnectionOpen(connection_hdl connectionHdl) {
    m_connection = connectionHdl;

    websocketpp::lib::error_code errorCode;
    auto client = m_webSocketServer.get_con_from_hdl(connectionHdl, errorCode);
    if (errorCode) {
        logError("onConnectionOpen", errorCode);
        return;
    }
    Logger::info("WebSocketServer::onConnectionOpen", "remoteHost:", client->get_remote_endpoint());

    m_observer->onConnectionOpened();
}

bool WebSocketServer::isReady() {
    return !m_connection.expired();
}

void WebSocketServer::setObserver(std::shared_ptr<MessagingServerObserverInterface> observer) {
    m_observer = observer;
}

void WebSocketServer::onConnectionClose(connection_hdl connectionHdl) {
    m_connection.reset();

    Logger::info("WebSocketServer::onConnectionClose");

    m_observer->onConnectionClosed();
}

void WebSocketServer::onMessage(connection_hdl connectionHdl, server::message_ptr messagePtr) {
    if (m_messageListener) {
        m_messageListener->onMessage(messagePtr->get_payload());
    } else {
        Logger::warn("WebSocketServer::onMessageFailed", "messageListener is null");
    }
}

bool WebSocketServer::onValidate(connection_hdl connectionHdl) {
    // As we currently don't support more than one connection in general and in GUIClient in particular reject all
    // connections if we already have one.
    bool result = m_connection.expired();
    if (!result) {
        Logger::warn("WebSocketServer::onValidate", "connection already open");
        asio::error_code errorCode;
        auto conn = m_webSocketServer.get_con_from_hdl(connectionHdl, errorCode);
        if (!errorCode) {
            conn->set_status(websocketpp::http::status_code::conflict);
        }
    }
    return result;
}

void WebSocketServer::logError(const std::string& method, websocketpp::lib::error_code error) {
    Logger::error(
        "WebSocketServer::logError", method, "errorCode:", error.value(), ", errorCategory:", error.category().name());
}

void WebSocketServer::logError(const std::string& method, const std::string& reason) {
    Logger::error("WebSocketServer::logError", method, ":", reason);
}
