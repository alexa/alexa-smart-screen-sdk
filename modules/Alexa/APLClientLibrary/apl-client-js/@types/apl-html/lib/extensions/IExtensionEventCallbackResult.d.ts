/*!
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
/**
 * Callback function for the resolution of an ExtensionEvent
 */
export interface IExtensionEventCallbackResult {
    onExtensionEventResult(succeeded: boolean): any;
}
