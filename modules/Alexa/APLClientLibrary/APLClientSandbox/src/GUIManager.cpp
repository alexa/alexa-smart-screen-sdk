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

#include <rapidjson/document.h>
#include <APLClientSandbox/Logger.h>
#include "APLClientSandbox/GUIManager.h"

static const std::chrono::milliseconds UPDATE_TICK_INTERVAL_MS{1000 / 60};  // 60 updates per second

std::shared_ptr<GUIManager> GUIManager::create(std::shared_ptr<WebSocketServer> server) {
    std::shared_ptr<GUIManager> guiManager(new GUIManager(server));
    guiManager->m_server->setMessageListener(guiManager);
    guiManager->m_server->setObserver(guiManager);
    guiManager->m_client->setGUIManager(guiManager);
    return guiManager;
}

GUIManager::GUIManager(std::shared_ptr<WebSocketServer> server) :
        m_server{std::move(server)},
        m_client{AplClientBridge::create()},
        m_connectionOpen{false} {
}

void GUIManager::onMessage(const std::string& payload) {
    Logger::debug("GUIManager::onMessage", payload);
    if (!m_connectionOpen) {
        Logger::error("GUIManager::onMessage", "Received message without active connection");
        return;
    }

    rapidjson::Document doc;
    if (doc.Parse(payload.c_str()).HasParseError()) {
        Logger::error("GUIManager::onMessage", "Failed to parse JSON");
        return;
    }

    if (!doc.HasMember("type") || !doc["type"].IsString()) {
        Logger::error("GUIManager::onMessage", "Missing type from JSON payload");
        return;
    }

    const std::string type = doc["type"].GetString();
    if (type == "renderDocument") {
        if (!doc.HasMember("document") || !doc["document"].IsString()) {
            Logger::error("GUIManager::onMessage", "renderDocument: Missing document from JSON payload");
            return;
        }

        if (!doc.HasMember("data") || !doc["data"].IsString()) {
            Logger::error("GUIManager::onMessage", "renderDocument: Missing data from JSON payload");
            return;
        }

        if (!doc.HasMember("viewports") || !doc["viewports"].IsString()) {
            Logger::error("GUIManager::onMessage", "renderDocument: Missing viewports from JSON payload");
            return;
        }

        const std::string document = doc["document"].GetString();
        const std::string data = doc["data"].GetString();
        const std::string viewports = doc["viewports"].GetString();

        m_client->renderDocument(document, data, viewports);
    } else if (type == "executeCommands") {
        if (!doc.HasMember("command") || !doc["command"].IsString()) {
            Logger::error("GUIManager::onMessage", "executeCommands: Missing command from JSON payload");
            return;
        }

        const std::string command = doc["command"].GetString();

        m_client->executeCommands(command);
    } else if (type == "apl") {
        if (!doc.HasMember("payload") || !doc["payload"].IsObject()) {
            Logger::error("GUIManager::onMessage", "apl: Missing payload from JSON payload");
            return;
        }

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        doc["payload"].Accept(writer);

        m_client->onMessage(sb.GetString());
    } else if (type == "resourceresponse") {
        if (!doc.HasMember("url") || !doc["url"].IsString()) {
            Logger::error("GUIManager::onMessage", "resourceresponse: Missing url from JSON payload");
            return;
        }

        if (!doc.HasMember("payload") || !doc["payload"].IsString()) {
            Logger::error("GUIManager::onMessage", "resourceresponse: Missing payload from JSON payload");
            return;
        }

        const std::string url = doc["url"].GetString();
        const std::string payload = doc["payload"].GetString();

        m_client->provideResource(url, payload);
    } else {
        Logger::error("GUIManager::onMessage", "Unknown message type", type);
    }
}

void GUIManager::onConnectionOpened() {
    m_connectionOpen = true;
    // Schedule the next update
    onUpdateComplete();
}

void GUIManager::onConnectionClosed() {
    m_connectionOpen = false;
    m_client->clearDocument();
}

void GUIManager::sendMessage(const Message& message) {
    sendMessage(message.get());
}

void GUIManager::sendMessage(const std::string& payload) {
    if (m_connectionOpen) {
        m_server->writeMessage(payload);
    } else {
        Logger::warn("GUIManager::sendMessage", "Attempted to send message without open connection");
    }
}

void GUIManager::onUpdateComplete() {
    // schedule the next update
    if (m_connectionOpen) {
        m_executor.submit([this]() {
            std::this_thread::sleep_for(UPDATE_TICK_INTERVAL_MS);
            m_client->updateTick();
        });
    }
}