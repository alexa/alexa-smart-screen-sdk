/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Callback function for the resolution of an ExtensionEvent
 */
export interface IExtensionEventCallbackResult {
    onExtensionEventResult(succeeded: boolean): any;
}
