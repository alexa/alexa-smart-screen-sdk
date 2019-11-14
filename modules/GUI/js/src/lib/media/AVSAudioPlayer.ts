/*!
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { AudioPlayer, IAudioEventListener, IAudioContextProvider, DefaultAudioContextProvider } from 'apl-client';
import { FocusManager } from '../focus/FocusManager';
import { FocusState } from '../focus/FocusState';
import { ChannelName } from '../focus/ChannelName';
import { ActivityTracker } from '../activity/ActivityTracker';

/**
 * Provides an implementation of the AudioPlayer which adds AVS focus support
 */
export class AVSAudioPlayer extends AudioPlayer {

    private focusManager : FocusManager;
    private focusToken : number;
    private playbackFocusResolver : { resolve : Function, reject : Function };
    private activityTracker : ActivityTracker;
    private activityToken : number;
    private audioContextProvider : IAudioContextProvider = new DefaultAudioContextProvider();

    public constructor(
      focusManager : FocusManager,
      activityTracker : ActivityTracker,
      eventListener : IAudioEventListener) {
        super(eventListener);
        this.focusManager = focusManager;
        this.activityTracker = activityTracker;
    }

    /**
     * Called when media has been downloaded and ready to play
     * @param id a uuid
     */
    public play(id : string) {
        Promise.all([this.audioContextProvider.getAudioContext(), this.acquireFocus()]).then((values) => {
            this.activityToken = this.activityTracker.recordActive('AVSAudioPlayer');
            const audioContext = values[0];
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
        super.onError(id, reason);
        this.releaseFocus();
        if (this.activityToken !== undefined) {
            this.activityTracker.recordInactive(this.activityToken);
            this.activityToken = undefined;
        }
    }

    private acquireFocus() : Promise<void> {
        return new Promise<void>(((resolve, reject) => {
            this.playbackFocusResolver = { resolve, reject };
            this.focusToken = this.focusManager.acquireFocus(ChannelName.DIALOG, {
                focusChanged : this.processFocusChanged.bind(this)
            });
        }));
    }

    private releaseFocus() {
        if (this.focusToken !== undefined) {
            this.focusManager.releaseFocus(this.focusToken);
        }
    }

    private processFocusChanged(focusState : FocusState) {
        // For dialog we do not allow background audio playback and will stop speech if this happens
        if (focusState !== FocusState.FOREGROUND) {
            if (this.playbackFocusResolver) {
                this.playbackFocusResolver.reject();
                this.playbackFocusResolver = undefined;
            } else {
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
