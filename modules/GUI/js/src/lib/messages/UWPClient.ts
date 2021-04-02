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
