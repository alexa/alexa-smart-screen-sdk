/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
import { PlaybackState } from './Resource';
/**
 * @ignore
 */
export interface IMediaEventListener {
    onEvent(event: PlaybackState): void;
}
