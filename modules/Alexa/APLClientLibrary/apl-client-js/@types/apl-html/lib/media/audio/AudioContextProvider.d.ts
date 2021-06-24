/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * Interface to provide AudioContext
 */
export interface IAudioContextProvider {
    /**
     * Get audio context on demand
     */
    getAudioContext(): Promise<AudioContext>;
    /**
     * Release an audio context
     *
     * @param audioContext an AudioContext object
     */
    releaseAudioContext(audioContext: AudioContext): Promise<void>;
}
/**
 * A default audio context provider
 */
export declare class DefaultAudioContextProvider implements IAudioContextProvider {
    private audioContext;
    getAudioContext(): Promise<AudioContext>;
    releaseAudioContext(audioContext: AudioContext): Promise<void>;
}
