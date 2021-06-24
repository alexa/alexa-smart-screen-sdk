/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
import { IBaseMarker } from '../audio/SpeechMarks';
/**
 * A callback interface to the initiator of audio requests
 * @ignore
 */
export interface IAudioEventListener {
    /**
     * Called when media has been downloaded (optionally parsed for markers)
     * (in response to prepare())
     * @param id a uuid
     */
    onPrepared(id: string): void;
    /**
     * Stream markers (if prepare was called with (<some url> markers: true))
     * The implementation is free to parse the entirity of the audio or parse
     * chunks as they are downloaded.
     * @param id a uuid
     * @param markers a list of IBaseMarker
     */
    onMarker(id: string, markers: IBaseMarker[]): void;
    /**
     * Called when media starts playing (in response to play())
     * @param id a uuid
     */
    onPlaybackStarted(id: string): void;
    /**
     * Called when media finishes playing
     * @param id a uuid
     */
    onPlaybackFinished(id: string): void;
    /**
     * Called to report an error for a given request
     * Used to report a download error or parse / playback error
     * @param id a uuid
     * @param reason an arbitrary string
     */
    onError(id: string, reason: string): void;
}
