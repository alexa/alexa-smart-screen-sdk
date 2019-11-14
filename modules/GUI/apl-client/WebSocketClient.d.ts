/**
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { APLClient } from "./APLClient";
/**
 * Implements the WebSocket version of an APLClient connection.
 */
export declare class WebSocketClient extends APLClient {
    private url;
    private ws;
    constructor(url: string);
    start(): Promise<void>;
    sendMessage(message: any): void;
    private onWebsocketMessage;
    private onWebsocketClose;
    private onWebsocketOpen;
    private onWebsocketError;
    private reconnect();
}
