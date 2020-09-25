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
#include <APLClientSandbox/GUIManager.h>
#include "APLClientSandbox/Logger.h"
#include "APLClientSandbox/WebSocketServer.h"

/**
 * WebSocket interface to listen on.
 */
static const std::string DEFAULT_WEBSOCKET_INTERFACE = "127.0.0.1";

/// WebSocket port to listen on.
static const int DEFAULT_WEBSOCKET_PORT = 8080;

class ConsoleWriter : public ILogWriter {
    void write(const std::string& message) {
        std::cout << message << std::endl;
    }
};

class WebSocketWriter : public ILogWriter {
private:
    std::shared_ptr<WebSocketServer> m_server;

public:
    WebSocketWriter(std::shared_ptr<WebSocketServer> server) : m_server{std::move(server)} {
    }

    void write(const std::string& message) {
        // m_server->writeMessage(message);
    }
};

std::shared_ptr<WebSocketServer> server;

void exitHandler(int) {
    if (server) {
        server->stop();
    }
}

int main(int argc, char** argv) {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exitHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    Logger::setDebugLogging(true);
    Logger::addSink(std::make_shared<ConsoleWriter>());

    server = std::make_shared<WebSocketServer>(DEFAULT_WEBSOCKET_INTERFACE, DEFAULT_WEBSOCKET_PORT);
    auto manager = GUIManager::create(server);
    server->start();
}