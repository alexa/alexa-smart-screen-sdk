/**
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
/**
 * MediaTrack defines the source and playback parameters
 * @ignore
 */
export interface IMediaSource {
    /**
     * The actual URL to load the video from
     */
    url: string;
    /**
     * Optional description of this source
     */
    description: string;
    /**
     * Duration of the track in milliseconds
     */
    duration: number;
    /**
     * Number of times to repeat. -1 is repeat forever
     */
    repeatCount: number;
    /**
     * Milliseconds from the start of the track to play from
     */
    offset: number;
}
