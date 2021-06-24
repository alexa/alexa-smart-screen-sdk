/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @ignore
 */
export declare type DownloadState = 'pending' | 'complete' | 'cancelled';
/**
 * @ignore
 */
export declare class Resource {
    private downloadState;
    private buffer;
    constructor();
    setDownloadState(downloadState: DownloadState): void;
    getDownloadState(): DownloadState;
    getBuffer(): ArrayBuffer;
    setBuffer(buffer: ArrayBuffer): void;
}
/**
 * Resource State of the audio
 * @ignore
 */
export declare enum ResourceState {
    PENDING = "pending",
    PREPARED = "prepared",
    PLAYING = "playing",
    PAUSED = "paused"
}
/**
 * Resource for play media command, command name
 * @ignore
 */
export declare enum ControlMediaCommandName {
    PLAY = "play",
    PAUSE = "pause",
    NEXT = "next",
    PREVIOUS = "previous",
    REWIND = "rewind",
    SEEK = "seek",
    SETTRACK = "setTrack"
}
/**
 * Playback state IDLE, PLAYING, ENDED, PAUSED, BUFFERING, ERROR
 * @ignore
 */
export declare enum PlaybackState {
    IDLE = "idling",
    LOADED = "loaded",
    PLAYING = "playing",
    ENDED = "ended",
    PAUSED = "paused",
    BUFFERING = "buffering",
    ERROR = "error"
}
