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

import { AudioPlayer, IAudioEventListener, IAudioContextProvider, DefaultAudioContextProvider } from 'apl-client';
import { IFocusManager } from '../focus/IFocusManager';
import { FocusState } from '../focus/FocusState';
import { ChannelName } from '../focus/ChannelName';
import { IActivityTracker } from '../activity/IActivityTracker';
import { AVSInterface } from '../focus/AVSInterface';
import { ContentType } from '../focus/ContentType';

/**
 * Provides an implementation of the AudioPlayer which adds AVS focus support
 */
export class AVSAudioPlayer extends AudioPlayer {

    private focusManager : IFocusManager;
    private focusToken : number;
    private playbackFocusResolver : { resolve : Function, reject : Function };
    private activityTracker : IActivityTracker;
    private activityToken : number;
    private audioContextProvider : IAudioContextProvider = new DefaultAudioContextProvider();
    private playing : boolean = false;
    private readonly avsInterface : AVSInterface;
    private readonly channelName : ChannelName;
    private readonly contentType : ContentType;

    public constructor(
        avsInterface : AVSInterface,
        channelName : ChannelName,
        contentType : ContentType,
        focusManager : IFocusManager,
        activityTracker : IActivityTracker,
        eventListener : IAudioEventListener) {
        super(eventListener);
        this.focusManager = focusManager;
        this.activityTracker = activityTracker;
        this.avsInterface = avsInterface;
        this.channelName = channelName;
        this.contentType = contentType;
    }

    /**
     * Called when media has been downloaded and ready to play
     * @param id a uuid
     */
    public play(id : string) {
        Promise.all([this.audioContextProvider.getAudioContext(), this.acquireFocus()]).then((values) => {
            this.activityToken = this.activityTracker.recordActive('AVSAudioPlayer');
            const audioContext = values[0];
            this.playing = true;
            this.playWithContext(id, audioContext);
        }).catch((reason? : any) => {
            console.log('AVSAudioPlayer:play failed with reason: ' + reason);
            this.releaseFocus();
            this.flush();
        });
    }

    /**
     * Called when media finishes playing
     * @param id a uuid
     */
    public onPlaybackFinished(id : string) : void {
        this.playing = false;
        super.onPlaybackFinished(id);
        this.releaseFocus();
        if (this.activityToken !== undefined) {
            this.activityTracker.recordInactive(this.activityToken);
            this.activityToken = undefined;
        }
    }

    /**
     * Called to report an error for a given request
     * Used to report a download error or parse / playback error
     * @param id a uuid
     * @param reason an arbitrary string
     */
    public onError(id : string, reason : string) : void {
        this.playing = false;
        super.onError(id, reason);
        this.releaseFocus();
        if (this.activityToken !== undefined) {
            this.activityTracker.recordInactive(this.activityToken);
            this.activityToken = undefined;
        }
    }

    /**
     * Called to stop currently playing audio and flush any pending decodes
     */
    public flush() : void {
        this.playing = false;
        super.flush();
    }

    private acquireFocus() : Promise<void> {
        if (this.playbackFocusResolver) {
            // We already had a focus token, if a promise exists reject the previous promise immediately
            this.playbackFocusResolver.reject();
        }
        return new Promise<void>(((resolve, reject) => {
            this.playbackFocusResolver = { resolve, reject };
            this.focusToken = this.focusManager.acquireFocus(this.avsInterface, this.channelName, this.contentType, {
                focusChanged : this.processFocusChanged.bind(this)
            });
        }));
    }

    private releaseFocus() {
        if (this.focusToken !== undefined) {
            this.focusManager.releaseFocus(this.focusToken);
            this.focusToken = undefined;
        }
    }

    private processFocusChanged(focusState : FocusState, token : number) {
        if (this.focusToken !== token) {
            // This was not the focus token we were expecting, ignore it
            return;
        }

        // For dialog we do not allow background audio playback and will stop speech if this happens
        if (focusState !== FocusState.FOREGROUND) {
            if (this.playbackFocusResolver) {
                this.playbackFocusResolver.reject();
                this.playbackFocusResolver = undefined;
            } else if (this.playing) {
                this.flush();
            }

            this.focusToken = undefined;
        } else {
            if (this.playbackFocusResolver) {
                this.playbackFocusResolver.resolve();
                this.playbackFocusResolver = undefined;
            }
        }
    }
}
