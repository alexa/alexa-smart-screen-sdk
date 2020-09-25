/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { IExtensionEventCallbackResult } from "./IExtensionEventCallbackResult";
/**
 * A common interface for managing extensions.
 */
export interface IExtensionManager {
    onExtensionEvent(uri: string, commandName: string, source: object, params: object, resultCallback: IExtensionEventCallbackResult): any;
}
