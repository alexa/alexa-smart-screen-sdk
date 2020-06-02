/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import {ILogger, LoggerFactory} from 'apl-client';
import {IBaseInboundMessage, IBaseOutboundMessage} from './messages';
import {IClient, IClientConfig, IOnMessageFunc} from './client';

/**
 * A UWP implementation for communicating with the GUI Server (MMSDK)
 * Outgoing messages, client to server, utilize the native method exposed to the
 * WebView container of the GUI Client.
 *
 * Incoming messages, server to client, utilize the invokeScriptAsync method of WebView
 */
export class UWPWebViewClient implements IClient {
    protected onMessage : IOnMessageFunc;
    protected logger : ILogger;

    constructor(config : IClientConfig) {
        this.onMessage = config.onMessage;
        this.logger = LoggerFactory.getLogger('UWPWebViewClient');

        let f = (msg : string) => {
            // this.logger.info('received message');
            let message : IBaseInboundMessage = undefined;
            try {
                message = JSON.parse(msg);
            } catch (e) {
                this.logger.error(`error parsing data: ${msg}`);
            }
            if (this.onMessage) {
                this.onMessage(message);
            }
        };

        (window as any).f = f;
    }

    public connect() {
        this.logger.debug('connect');
    }

    public disconnect() {
        this.logger.debug('disconnect');
    }

    public sendMessage(message : IBaseOutboundMessage) {
        const json = JSON.stringify(message);
        if (this.sendRawMessage(json)) {
            this.logger.info(`message sent, type: ${message.type}`);
        } else {
            this.logger.error('message could not be delivered');
        }
    }

    public sendRawMessage(rawMessage : string) : boolean {
        (window as any).class1.stringFromJavaScript(rawMessage);
        return true;
    }

    public isConnected() : boolean {
        return true;
    }
}
