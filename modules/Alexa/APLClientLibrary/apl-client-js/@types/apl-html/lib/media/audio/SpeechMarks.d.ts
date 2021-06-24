/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * The type of speech mark
 * @ignore
 */
export declare type SpeechMarkType = 'visime' | 'word' | 'sentence';
/**
 * Base type
 * @ignore
 */
export interface IBaseMarker {
    /**
     * Marker type
     */
    type: SpeechMarkType;
    /**
     * Time offset in milliseconds
     */
    time: number;
    /**
     * Value of the marker (defined by type)
     */
    value: string;
}
/**
 * Extended type for word / sentence marker type
 * @ignore
 */
export interface IFragmentMarker extends IBaseMarker {
    /**
     * Start index in the sentence of this fragment
     */
    start: number;
    /**
     * End index ( +1 ) in the sentence of this fragment
     */
    end: number;
}
