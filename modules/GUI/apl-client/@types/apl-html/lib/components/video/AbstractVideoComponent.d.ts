/**
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import APLRenderer from '../../APLRenderer';
import { AudioTrack } from '../../enums/AudioTrack';
import { CommandControlMedia } from '../../enums/CommandControlMedia';
import { PropertyKey } from '../../enums/PropertyKey';
import { VideoScale } from '../../enums/VideoScale';
import { IMediaEventListener } from '../../media/IMediaEventListener';
import { IMediaSource } from '../../media/IMediaSource';
import { PlaybackState } from '../../media/Resource';
import { Component, FactoryFunction, IComponentProperties } from '../Component';
/**
 * @ignore
 */
export interface IVideoProperties extends IComponentProperties {
    [PropertyKey.kPropertyAudioTrack]: AudioTrack;
    [PropertyKey.kPropertyAutoplay]: boolean;
    [PropertyKey.kPropertyScale]: VideoScale;
    [PropertyKey.kPropertySource]: IMediaSource | IMediaSource[];
}
/**
 * @ignore
 */
export declare abstract class AbstractVideoComponent extends Component<IVideoProperties>  implements IMediaEventListener {
    protected constructor(renderer: APLRenderer, component: APL.Component, factory: FactoryFunction, parent?: Component);
    abstract onEvent(event: PlaybackState): void;
    abstract playMedia(source: IMediaSource | IMediaSource[], audioTrack: AudioTrack): any;
    abstract controlMedia(operation: CommandControlMedia, optionalValue: number): any;
    abstract play(waitForFinish?: boolean): any;
    abstract pause(): any;
    abstract next(): any;
    abstract previous(): any;
    abstract rewind(): any;
    abstract seek(offset: number): any;
    abstract setTrack(trackIndex: number): any;
    protected abstract setScale(scale: VideoScale): any;
    protected abstract setAudioTrack(audioTrack: AudioTrack): any;
    protected abstract setSource(source: IMediaSource | IMediaSource[]): any;
    private setScaleFromProp;
    private setAudioTrackFromProp;
    private setSourceFromProp;
}
