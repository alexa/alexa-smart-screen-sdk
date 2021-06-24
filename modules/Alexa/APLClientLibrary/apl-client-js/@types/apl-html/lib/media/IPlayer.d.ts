/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { PlaybackState } from './Resource';
/**
 * @ignore
 */
export interface IPlayer {
    /**
     * Load media metadata, and preload the content as well
     * @param id a uuid
     * @param url (optional) URL of the media source
     */
    load(id: string, url?: string, decodeMarkers?: boolean): Promise<void>;
    /**
     * Play a previously prepared media resource
     * @param id a uuid
     * @param url (optional) URL of the media source
     */
    play(id: string, url?: string, offset?: number): Promise<void>;
    /**
     * Pause the current playing video
     */
    pause(): void;
    /**
     * Set volume of media
     * @param volume [0, 1] for volume.
     */
    setVolume(volume: number): void;
    /**
     * Stop media, clear prepared resources and cancel any in-flight downloads
     */
    flush(): void;
    /**
     * @returns state of the media playback - IDLE, PLAYING, ENDED, PAUSED, BUFFERING, ERROR
     */
    getMediaState(): PlaybackState;
    /**
     * @returns id of the currently sourced media
     */
    getMediaId(): string;
}
