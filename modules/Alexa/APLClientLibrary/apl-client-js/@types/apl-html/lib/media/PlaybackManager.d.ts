/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { IMediaSource } from './IMediaSource';
/**
 * Media resource state
 * @ignore
 */
export interface IMediaResource {
    toRepeat: number;
    description: string;
    offset: number;
    repeatCount: number;
    trackIndex: number;
    url: string;
    id: string;
    loaded: boolean;
    duration: number;
}
/**
 * @ignore
 */
export declare class PlaybackManager {
    private current;
    private resources;
    setup(sources: IMediaSource | IMediaSource[]): void;
    getCurrentIndex(): number;
    getTrackCount(): number;
    getCurrent(): IMediaResource;
    next(): IMediaResource;
    previous(): IMediaResource;
    setCurrent(index: number): void;
    hasNext(): boolean;
    repeat(): boolean;
    private addToPlaylist(index, track);
    private reset();
}
