/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { IMediaEventListener } from '../IMediaEventListener';
import { PlaybackState } from '../Resource';
import { IPlayer } from '../IPlayer';
/**
 * @ignore
 */
export declare class VideoPlayer implements IPlayer {
    protected player: HTMLVideoElement;
    protected eventListener: IMediaEventListener;
    protected playbackState: PlaybackState;
    private playerEndTime;
    constructor(eventListener: IMediaEventListener);
    configure(parent: HTMLElement, scale: 'contain' | 'cover'): void;
    applyCssShadow: (shadowParams: string) => void;
    load(id: string, url: string): Promise<void>;
    play(id: string, url: string, offset: number): Promise<void>;
    pause(): void;
    mute(): void;
    unmute(): void;
    setVolume(volume: number): void;
    flush(): void;
    setCurrentTime(offsetInSecond: number): void;
    setEndTime(endTimeInSecond: number): void;
    getCurrentPlaybackPosition(): number;
    getDuration(): number;
    getMediaState(): PlaybackState;
    getMediaId(): string;
    private sendPlaying();
    private onVideoTimeUpdated();
}
