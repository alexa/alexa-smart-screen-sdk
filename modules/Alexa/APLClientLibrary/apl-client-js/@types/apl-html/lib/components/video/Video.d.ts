/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import APLRenderer from '../../APLRenderer';
import { AudioTrack } from '../../enums/AudioTrack';
import { CommandControlMedia } from '../../enums/CommandControlMedia';
import { VideoScale } from '../../enums/VideoScale';
import { IMediaSource } from '../../media/IMediaSource';
import { IMediaResource, PlaybackManager } from '../../media/PlaybackManager';
import { PlaybackState } from '../../media/Resource';
import { HLSVideoPlayer as VideoPlayer } from '../../media/video/HLSVideoPlayer';
import { Component, FactoryFunction } from '../Component';
import { AbstractVideoComponent } from './AbstractVideoComponent';
/**
 * @ignore
 */
export declare class Video extends AbstractVideoComponent {
    protected player: VideoPlayer;
    protected playbackManager: PlaybackManager;
    protected currentMediaResource: IMediaResource;
    protected currentMediaState: APL.IMediaState;
    protected audioTrack: AudioTrack;
    private videoState;
    private playPromise;
    private pausePromise;
    private loadPromise;
    private playCallback;
    private pauseCallback;
    private loadCallback;
    private isSettingSource;
    private fromEvent;
    private trackCurrentTime;
    constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    onEvent(event: PlaybackState): void;
    protected applyCssShadow: (shadowParams: string) => void;
    playMedia(source: IMediaSource | IMediaSource[], audioTrack: AudioTrack): Promise<void>;
    controlMedia(operation: CommandControlMedia, optionalValue: number): Promise<void>;
    play(waitForFinish?: boolean): Promise<void>;
    pause(): Promise<void>;
    next(): Promise<void>;
    previous(): Promise<void>;
    rewind(): Promise<void>;
    seek(offset: number): Promise<void>;
    setTrack(trackIndex: number): Promise<void>;
    protected setScale(scale: VideoScale): void;
    protected setAudioTrack(audioTrack: AudioTrack): void;
    protected setSource(source: IMediaSource | IMediaSource[]): Promise<void>;
    protected setTrackCurrentTime(trackCurrentTime: number): void;
    protected setTrackIndex(trackIndex: number): void;
    protected updateMediaState(): void;
    private resetPausePromise();
    private resetPlayPromise();
    private resetLoadPromise();
    private ensureLoaded();
    /**
     * Return if the video should be paused when seeking to an offset.
     * The play/pause should depend on kPropertyAutoplay at initial load - offset == 0.
     * The play/pause should depend on kPropertyTrackPaused once video has been played - offset > 0.
     *
     * @param seekOffset
     * @private
     */
    private shouldPauseAtSeek(seekOffset);
    destroy(): void;
}
