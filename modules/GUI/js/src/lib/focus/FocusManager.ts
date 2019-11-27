/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

import { IFocusResource } from './FocusResource';
import { IFocusBridge } from './IFocusBridge';
import { IChannelObserver } from './IChannelObserver';
import { ChannelName } from './ChannelName';
import { FocusState } from './FocusState';

/**
 * Simple data structure to hold information about focus requestor.
 *
 * @interface
 * @export
 */
export interface IRequesterInfo {
    channelName : ChannelName;
    channelObserver : IChannelObserver;
    focusResource? : IFocusResource;
    resourcePromise? : Promise<void>;
}

/**
 * FocusManager implementation. Handles requests from commands/components to AVS SDK FocusManager in order to acquire
 * or release Visual or Audio channels.
 *
 * @class
 * @exports
 */
export class FocusManager {
    private tokenToInfoMap : Map<number, IRequesterInfo> = new Map();
    private currentToken = 0;
    private focusBridge : IFocusBridge;

    /**
     * @param focusBridge Focus bridge to be provided from JS shim in C++ SDK. Can be undefined for web renderer test.
     */
    constructor(focusBridge : IFocusBridge) {
        this.focusBridge = focusBridge;
    }

    /**
     * Acquire channel from AVS SDK.
     *
     * @param channelName Name of the channel to acquire.
     * @param observer Channel state observer.
     * @returns Assigned requestor token.
     */
    public acquireFocus(channelName : ChannelName, observer : IChannelObserver) : number {
        const token = this.currentToken++;
        this.tokenToInfoMap.set(token, {channelName, channelObserver : observer});
        this.focusBridge.acquireFocus(channelName, token);
        return token;
    }

    /**
     * Release channel to AVS SDK. It will use same observer that was provided to acquireFocus to report result.
     *
     * @param token Requestor token received while acquiring channel.
     */
    public releaseFocus(token : number) {
        const requesterInfo : IRequesterInfo = this.tokenToInfoMap.get(token);
        if (requesterInfo) {
            this.focusBridge.releaseFocus(requesterInfo.channelName, token);
        } else {
            console.warn(`releaseFocus request failed for non-existing token. token: ${token}`);
        }
    }

    /**
     * Process AVS SDK FocusManager response on acquire/release request.
     *
     * @param token Requestor token.
     * @param result Request processing result.
     */
    public processFocusResponse(token : number, result : any) {
        // No actual need to process it at the moment, could be used for retries.
    }

    /**
     * Process AVS SDK Channel state change.
     *
     * @param token Requestor token.
     * @param focusStateString Acquired or released channel state.
     */
    public processFocusChanged(
      token : number,
      focusStateString : string) {
        const focusState : FocusState = FocusState[focusStateString as keyof typeof FocusState];
        const requesterInfo = this.tokenToInfoMap.get(token);
        if (requesterInfo) {
            requesterInfo.channelObserver.focusChanged(focusState, token);

            // If NONE, then it was released. Clean up info here.
            if (FocusState.NONE === focusState) {
                this.tokenToInfoMap.delete(token);
            }
        } else {
            console.warn(`processFocusChanged. token: ${token}` +
                'received change for non-existing requestor.');
        }
    }

    /**
     * Reset FocusManager state. Release all channels held and clear the list.
     */
    public reset() {
        this.tokenToInfoMap.forEach((value : IRequesterInfo, key : number) => this.releaseFocus(key));
        this.tokenToInfoMap.clear();
    }
}
