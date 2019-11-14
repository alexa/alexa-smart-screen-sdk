/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

/**
 * Binding interface for JS Shim in AVS SDK.
 *
 * @export
 * @interface IFocusBridge
 */
export interface IFocusBridge {
    acquireFocus(channelName : string, token : number) : void;
    releaseFocus(channelName : string, token : number) : void;
}
