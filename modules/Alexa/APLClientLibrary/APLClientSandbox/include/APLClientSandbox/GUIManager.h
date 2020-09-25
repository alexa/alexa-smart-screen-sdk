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

#ifndef APLCLIENTSANDBOX_INCLUDE_GUIMANAGER_H_
#define APLCLIENTSANDBOX_INCLUDE_GUIMANAGER_H_

#include <memory>
#include "WebSocketServer.h"
#include "AplClientBridge.h"
#include "Message.h"

class AplClientBridge;

class GUIManager
        : public MessageListenerInterface
        , public MessagingServerObserverInterface
        , public std::enable_shared_from_this<GUIManager> {
public:
    /// Virtual destructor
    virtual ~GUIManager() = default;

    /**
     * Creates a GUIManager
     * @param server The websocket server
     * @return pointer to the created GUIManager
     */
    static std::shared_ptr<GUIManager> create(std::shared_ptr<WebSocketServer> server);

    /// @name MessageListerInterface functions
    /// @{
    void onMessage(const std::string& payload) override;
    /// @}

    /// @name MessagingServerObserverInterface functions
    /// @{
    void onConnectionOpened() override;

    void onConnectionClosed() override;
    /// @}

    /**
     * Sends a message to the GUI
     * @param message The message object to send
     */
    void sendMessage(const Message& message);

    /**
     * Sends a message to the GUI
     * @param payload The string payload to send
     */
    void sendMessage(const std::string& payload);

    /**
     * Should be called once an update loop has finished executing - will queue the next update
     */
    void onUpdateComplete();

private:
    /// Private constructor
    GUIManager(std::shared_ptr<WebSocketServer> server);

    /// The WebSocketServer
    std::shared_ptr<WebSocketServer> m_server;

    /// The AplClientBridge
    std::shared_ptr<AplClientBridge> m_client;

    /// Indicates whether a websocket connection is currently open
    bool m_connectionOpen;

    /// The execution thread
    Executor m_executor;
};

#endif  // APLCLIENTSANDBOX_INCLUDE_GUIMANAGER_H_
