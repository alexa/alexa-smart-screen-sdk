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

import { LogTransport, JSLogLevel } from 'apl-client';
import { IClient } from './client';
import { ILogEventMessage } from './messages';

/// Simple class to handle log redirection to SDK through messages.
export class SDKLogTransport {
    /// Connection client to use.
    private static client : IClient;

    /**
     * Initialize log transport.
     *
     * @param client Client to use.
     */
    public static initialize(client : IClient) {
        this.client = client;
    }

    /**
     * Callback function to be used as log transport. See @LogTransport.
     *
     * @param level Log level (trace | debug | info | warn | error).
     * @param loggerName Name of the logger. Usually same as component/class name.
     * @param message Actual log message.
     */
    public static logFunction : LogTransport = (level : JSLogLevel, loggerName : string, message : string) => {
        if (SDKLogTransport.client && SDKLogTransport.client.isConnected()) {
            SDKLogTransport.sendLogEvent(level, loggerName, message);
        } else {
            (console as any)[level](`${level[0].toUpperCase()} ${loggerName}: ${message}`);
        }
    }

    /**
     * Internal function to send messages to SDK.
     *
     * @param level Log level to use.
     * @param component Reporting component name.
     * @param message Log message.
     */
    private static sendLogEvent(level : JSLogLevel, component : string, message : string) {
        const logEventMessage : ILogEventMessage = {
            type : 'logEvent',
            level,
            component,
            message
        };

        // We use same client that could log on sendMessage so we should use raw send instead to avoid recursion.
        if (!this.client.sendRawMessage(JSON.stringify(logEventMessage))) {
            // Can't and shouldn't happen as logger actually checks for connection being in place, so log directly.
            console.warn('Trying to send logs to SDK while no connection present.');
        }
    }
}
