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

import {ChannelName} from './ChannelName';
import {IChannelObserver} from './IChannelObserver';
import {AVSInterface} from './AVSInterface';
import {ContentType} from './ContentType';

/**
 * Interface for manager handling audio channel focus acquisition.
 * @see https://developer.amazon.com/docs/alexa-voice-service/focus-management.html
 *
 * @interface
 * @exports
 */
export interface IFocusManager {
    acquireFocus(
        avsInterface : AVSInterface,
        channelName : ChannelName,
        contentType : ContentType,
        observer : IChannelObserver) : number;
    releaseFocus(token : number) : void;
    processFocusResponse(token : number, result : any) : void;
    processFocusChanged(token : number, focusStateString : string) : void;
    reset() : void;
}
