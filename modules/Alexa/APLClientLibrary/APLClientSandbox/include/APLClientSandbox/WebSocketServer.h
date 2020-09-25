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

#ifndef APLCLIENTSANDBOX_INCLUDE_WEBSOCKETSERVER_H_
#define APLCLIENTSANDBOX_INCLUDE_WEBSOCKETSERVER_H_

#include <string>
#include <set>

#include <websocketpp/server.hpp>

#include "WebSocketConfig.h"

/*
 Implementation Notes
 --------------------

 This implementation is loosely taken from the example code within the WebSocketPP library.

 The main idea behind ASIO is that all requests to write/read are asynchronous, generally any request to read/write
 is provided a callback, which is called when the the request is complete. In cases where the input is required for e.g.
 for read, ASIO returns control flow immediately, calling the supplied callback at some later time.

 Top-level sequence expressed serially, session operations are handled by the WebSocketPP library:

 Setup listener socket on supplied port
 Wait for connection
 Get notified of new connection - add it to the set of connections

 When data is received from a client, onMessage will be called with the message payload

 When sending a message to clients, the data is sent to each connection which is currently open.

 When a client disconnects for any reason, onConnectionClose is called - the connection is removed from the
  set of connections

 Additional notes
 ----------------
 This implementation supports multiple clients; writes will be fanned out to all clients, reads are accepted
 from any client. The connection_hdl descriptor identifies the client which is opening/closing a connection or
 sending a message.
*/

/**
 * An interface that listens to incoming messages from arbitrary sources
 */
class MessageListenerInterface {
public:
    /**
     * Called when a new message is available on the arbitrary source channel
     *
     * @note Blocking in this handler will block delivery of further messages.
     * @param payload an arbitrary string
     */
    virtual void onMessage(const std::string& payload) = 0;
};

/**
 * Observe @c MessagingServer events.
 */
class MessagingServerObserverInterface {
public:
    /**
     * A new connection to the server has been opened.
     */
    virtual void onConnectionOpened() = 0;

    /**
     * A connection to the server has been closed.
     */
    virtual void onConnectionClosed() = 0;
};

/**
 * A @c MessagingServerInterface implementation using WebSocket.
 * The @c start method is blocking.
 */
class WebSocketServer {
public:
    /**
     * Constructor.
     *
     * @param interface Network interface to bind / listen.
     * @param port Port to bind / listen.
     */
    WebSocketServer(const std::string& interface, unsigned short port);

    bool start();
    void writeMessage(const std::string& payload);
    void setMessageListener(std::shared_ptr<MessageListenerInterface> messageListener);
    void stop();
    bool isReady();
    void setObserver(std::shared_ptr<MessagingServerObserverInterface> observer);

    virtual ~WebSocketServer() = default;

private:
    typedef websocketpp::server<WebSocketConfig> server;
    using connection_hdl = websocketpp::connection_hdl;

    /**
     * Callback from WebSocket server when a connection is opened.
     *
     * @param connectionHdl Connection handler.
     */
    void onConnectionOpen(connection_hdl connectionHdl);

    /**
     * Callback from WebSocket server when a connection is closed.
     *
     * @param connectionHdl Connection handler.
     */
    void onConnectionClose(connection_hdl connectionHdl);

    /**
     * Callback from WebSocket server when a message is received.
     *
     * @param connectionHdl Connection handler.
     * @param messagePtr Pointer to the message.
     */
    void onMessage(connection_hdl connectionHdl, server::message_ptr messagePtr);

#ifdef ENABLE_WEBSOCKET_SSL
    /**
     * Callback from WebSocket server when tls is initialized - used to set tls options
     */
    std::shared_ptr<asio::ssl::context> onTlsInit();
#endif

    /**
     * Callback from WebSocket after handshake has been received and processed but before it has been accepted.
     *
     * @param connectionHdl Connection handler.
     * @return true on accept, false on reject.
     */
    bool onValidate(connection_hdl connectionHdl);

    static void logError(const std::string& method, websocketpp::lib::error_code error);

    static void logError(const std::string& method, const std::string& reason);

    /// Indicates whether the server was successfully initialised
    std::atomic_bool m_initialised{false};

    /// The websocket server instance
    server m_webSocketServer;

    /// Reference to a message listener to be called when a new message is received
    std::shared_ptr<MessageListenerInterface> m_messageListener;

    /// Reference to current session
    connection_hdl m_connection;

    /// The websocket ssl certificate authority file
    std::string m_certificateAuthorityFile;

    /// The websocket ssl certificate chain file
    std::string m_certificateFile;

    /// The websocket ssl private key file
    std::string m_privateKeyFile;

    /// The server observer.
    std::shared_ptr<MessagingServerObserverInterface> m_observer;
};

#endif  // APLCLIENTSANDBOX_INCLUDE_WEBSOCKETSERVER_H_
