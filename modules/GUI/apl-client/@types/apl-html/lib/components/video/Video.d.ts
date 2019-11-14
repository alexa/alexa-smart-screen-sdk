/**
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
    private fromEvent;
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
    protected setSource(source: IMediaSource | IMediaSource[]): void;
    protected updateMediaState(): void;
    private resetPausePromise();
    private resetPlayPromise();
    private resetLoadPromise();
    private ensureLoaded();
}
