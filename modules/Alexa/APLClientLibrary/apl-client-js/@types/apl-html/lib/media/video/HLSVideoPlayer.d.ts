/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { IMediaEventListener } from '../IMediaEventListener';
import { VideoPlayer } from './VideoPlayer';
/**
 * @ignore
 */
export declare class HLSVideoPlayer extends VideoPlayer {
    private hlsPlayer;
    constructor(eventListener: IMediaEventListener);
    load(id: string, url: string): Promise<void>;
    play(id: string, url: string, offset: number): Promise<void>;
    private playHls(url, loadMetadataOnly?);
}
