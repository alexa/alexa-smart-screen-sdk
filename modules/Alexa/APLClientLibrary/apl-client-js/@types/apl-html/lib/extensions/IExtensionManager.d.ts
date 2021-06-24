/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { IExtensionEventCallbackResult } from './IExtensionEventCallbackResult';
/**
 * A common interface for managing extensions.
 */
export interface IExtensionManager {
    onExtensionEvent(uri: string, event: APL.Event, commandName: string, source: object, params: object, resultCallback: IExtensionEventCallbackResult): any;
    onDocumentRender(rootContext: APL.Context, content: APL.Content): any;
    configureExtensions(extensionConfiguration: any): any;
    onMessageReceived(uri: string, payload: string): any;
    onDocumentFinished(): any;
    resetRootContext(): any;
}
