/**
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import {
    AbstractVideoComponent,
    APLRenderer,
    AudioTrack,
    Component,
    FactoryFunction,
    IVideoFactory,
    PlaybackState,
    Video
} from 'apl-client';
import { FocusManager } from '../focus/FocusManager';
import { ChannelName } from '../focus/ChannelName';
import { FocusState } from '../focus/FocusState';
import { ActivityTracker } from '../activity/ActivityTracker';

/**
 * Factory class which creates AVSVideo objects
 */
export class AVSVideoFactory implements IVideoFactory {
    private focusManager : FocusManager;
    private activityTracker : ActivityTracker;

    constructor(focusManager : FocusManager, activityTracker : ActivityTracker) {
        this.focusManager = focusManager;
        this.activityTracker = activityTracker;
    }

    public create(renderer : APLRenderer,
                  component : APL.Component,
                  factory : FactoryFunction,
                  parent? : Component) : AbstractVideoComponent {
        return new AVSVideo(this.focusManager, this.activityTracker, renderer, component, factory, parent);
    }
}

/**
 * A Video component which adds AVS focus support
 */
export class AVSVideo extends Video {
    private focusManager : FocusManager;
    private focusToken : number;
    private playbackFocusResolver : { resolve : Function, reject : Function };
    private activityTracker : ActivityTracker;
    private activityToken : number;

    constructor(focusManager : FocusManager,
                activityTracker : ActivityTracker,
                renderer : APLRenderer,
                component : APL.Component,
                factory : FactoryFunction,
                parent? : Component) {
        super(renderer, component, factory, parent);
        this.focusManager = focusManager;
        this.activityTracker = activityTracker;
    }

    /**
     * A Playback event has occurred
     * @param event The new playback state
     */
    public onEvent(event : PlaybackState) : void {
        super.onEvent(event);

        switch (event) {
            case PlaybackState.PAUSED: // FALLTHROUGH
            case PlaybackState.ENDED: // FALLTHROUGH
            case PlaybackState.ERROR: // FALLTHROUGH
            case PlaybackState.IDLE:
                this.releaseFocus();
                if (this.activityToken !== undefined) {
                    this.activityTracker.recordInactive(this.activityToken);
                    this.activityToken = undefined;
                }
                break;

            case PlaybackState.PLAYING:
                this.activityToken = this.activityTracker.recordActive(`AVSVideo-${this.id}`);
                break;
            case PlaybackState.BUFFERING: // FALLTHROUGH
            case PlaybackState.LOADED: // FALLTHROUGH
            default:
                break;
        }
    }

    /**
     * Function to start video playback
     */
    public async play(waitForFinish : boolean = false) {
        if (this.audioTrack === AudioTrack.kAudioTrackNone) {
            // Focus is not required if there is no audio track
            this.player.mute();

            await super.play(waitForFinish);
        } else {
            const focusChannel = this.audioTrack === AudioTrack.kAudioTrackBackground ?
                ChannelName.CONTENT : ChannelName.DIALOG;

            await this.acquireFocus(focusChannel)
                .then(() => super.play(waitForFinish));
        }
    }

    private acquireFocus(channel : ChannelName) : Promise<void> {
        return new Promise<void>(((resolve, reject) => {
            this.playbackFocusResolver = {
                resolve : () => {
                    resolve();
                    this.playbackFocusResolver = undefined;
                },
                reject : () => {
                    reject();
                    this.playbackFocusResolver = undefined;
                }
            };
            this.focusToken = this.focusManager.acquireFocus(channel, {
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
        switch (focusState) {
            case FocusState.NONE:
                if (this.playbackFocusResolver) {
                    console.error('Invalid focus transition, initial focus state was NONE');
                    this.playbackFocusResolver.reject();
                } else {
                    this.pause();
                }

                this.focusToken = undefined;
                break;
            case FocusState.FOREGROUND:
                this.player.unmute();

                if (this.playbackFocusResolver) {
                    this.playbackFocusResolver.resolve();
                }
                break;
            case FocusState.BACKGROUND:
                // Do not start playback with only background focus as it causes an undesirable user experience
                this.player.mute();
                break;
            default:
                console.error('Unknown focus state');
                break;
        }
    }
}
