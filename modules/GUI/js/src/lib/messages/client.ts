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

import { IBaseOutboundMessage, IAlexaStateChangedMessage, AlexaState, IBaseInboundMessage } from './messages';
import { ILogger, LoggerFactory } from 'apl-client';

/// Max backoff value for reconnect attempts.
const MAX_BACKOFF = 10;

/// Received message handling callback definition.
export interface IOnMessageFunc {
    (message : IBaseInboundMessage) : void;
}

/// GUI server client configuration.
export interface IClientConfig {
    host : string;
    port : number;
    onMessage : IOnMessageFunc;
    insecure : boolean;
}

/**
 * GUI Server connection client.
 */
export interface IClient {
    /**
     * Connect to server.
     */
    connect() : void;

    /**
     * Disconnect from server.
     */
    disconnect() : void;

    /**
     * Send message using current client.
     *
     * @param message message to send.
     */
    sendMessage(message : IBaseOutboundMessage) : void;

    /**
     * Send message using current client. Message is in raw form. No logs emitted.
     *
     * @param rawMessage message to send.
     */
    sendRawMessage(rawMessage : string) : boolean;

    /**
     * Get current client state. true if connected, false otherwise.
     */
    isConnected() : boolean;
}

/**
 * Default client implementation based on Websockets.
 */
export class Client implements IClient {
    protected connectRequested : boolean = false;
    protected connected : boolean = false;
    protected timerId : number;
    protected backoff : number;
    protected url : string;
    protected ws : WebSocket;
    protected onMessage : IOnMessageFunc;
    protected logger : ILogger;

    protected onclose(ev : CloseEvent) : void {
        this.connected = false;
        this.logger.debug('onclose');
        this.ws = undefined;
        if (this.connectRequested) {
            this.logger.info('Trying to reconnect.');
            this.backoff = this.backoff === 0 ? 2 : Math.min(this.backoff * 2, MAX_BACKOFF);
            this.scheduleConnect();
        } else {
            this.logger.info('closed');
        }
    }

    protected onopen(ev : Event) : void {
        this.logger.debug('onopen');
        this.backoff = 0;
        this.connected = true;
    }

    protected onerror(ev : Event) : void {
        this.ws.close();
        // Force error state on connection lost
        this.logger.error('error');
        const stateChangedMessage : IAlexaStateChangedMessage = {type: 'alexaStateChanged', state: AlexaState.UNKNOWN};
        this.onMessage(stateChangedMessage);
    }

    constructor(config : IClientConfig) {
        const protocol = config.insecure ? 'ws://' : 'wss://';
        this.url = protocol + config.host + ':' + config.port;
        this.onMessage = config.onMessage;
        this.logger = LoggerFactory.getLogger('WSClient');
    }

    protected wsOnMessage(event : MessageEvent) {
        this.logger.info('received message');
        let message : IBaseInboundMessage = undefined;
        try {
            message = JSON.parse(event.data);
        } catch (e) {
            this.logger.error(`error parsing data: ${event.data}`);
        }

        if (this.onMessage) {
            this.onMessage(message);
        }
    }

    protected scheduleConnect() {
        const that = this;

        const callback = () => {
            that.timerId = undefined;
            that.ws = new WebSocket(that.url);
            that.ws.onmessage = that.wsOnMessage.bind(that);
            that.ws.onclose = that.onclose.bind(that);
            that.ws.onopen = that.onopen.bind(that);
            that.ws.onerror = that.onerror.bind(that);
        };
        this.logger.info(`Scheduling connection to: ${this.url} with backoff ${this.backoff}`);
        this.timerId = window.setTimeout(callback, this.backoff * 1000);
    }

    public connect() {
        this.logger.debug('connect');
        this.connectRequested = true;
        if (this.ws === undefined && this.timerId === undefined) {
            this.backoff = 0;
            this.scheduleConnect();
        }
    }

    public disconnect() {
        this.logger.debug('disconnect');
        this.connectRequested = false;
        if (this.ws) {
            this.ws.close();
        } else if (this.timerId) {
            window.clearTimeout(this.timerId);
            this.timerId = undefined;
        }
    }

    public sendMessage(message : IBaseOutboundMessage) : void {
        const json = JSON.stringify(message);
        if (this.sendRawMessage(json)) {
            this.logger.info(`message sent, type: ${message.type}`);
        } else {
            this.logger.error('message could not be delivered');
        }

    }

    public sendRawMessage(rawMessage : string) : boolean {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(rawMessage);
            return true;
        }
        return false;
    }

    public isConnected() : boolean {
        return this.connected;
    }
}
