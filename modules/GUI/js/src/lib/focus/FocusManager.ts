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

'use strict';

import { IFocusResource } from './FocusResource';
import { IFocusBridge } from './IFocusBridge';
import { IChannelObserver } from './IChannelObserver';
import { ChannelName } from './ChannelName';
import { FocusState } from './FocusState';
import { IFocusManager } from './IFocusManager';
import { AVSInterface } from './AVSInterface';
import { ContentType } from './ContentType';

/**
 * Simple data structure to hold information about focus requestor.
 *
 * @interface
 * @export
 */
export interface IRequesterInfo {
    avsInterface : AVSInterface;
    channelName : ChannelName;
    contentType : ContentType;
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
export class FocusManager implements IFocusManager {
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
     * @param avsInterface Name of the avs interface to report as having focus.
     * @param channelName Name of the channel to acquire.
     * @param contentType Type of content acquiring focus.
     * @param observer Channel state observer.
     * @returns Assigned requester token.
     */
    public acquireFocus(
        avsInterface : AVSInterface,
        channelName : ChannelName,
        contentType : ContentType,
        observer : IChannelObserver) : number {
        const token = this.currentToken++;
        this.tokenToInfoMap.set(token, {avsInterface, channelName, contentType, channelObserver : observer});
        this.focusBridge.acquireFocus(avsInterface, channelName, contentType, token);
        return token;
    }

    /**
     * Release channel to AVS SDK. It will use same observer that was provided to acquireFocus to report result.
     *
     * @param token Requester token received while acquiring channel.
     */
    public releaseFocus(token : number) {
        const requesterInfo : IRequesterInfo = this.tokenToInfoMap.get(token);
        if (requesterInfo) {
            this.focusBridge.releaseFocus(requesterInfo.avsInterface, requesterInfo.channelName, token);
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
     * @param token Requester token.
     * @param focusStateString Acquired or released channel focus state.
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
                'received change for non-existing requester.');
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
