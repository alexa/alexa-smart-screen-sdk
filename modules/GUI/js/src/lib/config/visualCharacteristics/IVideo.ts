/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

export type VideoCodecs =
    'H_264_41'
    | 'H_264_42';

export interface IVideo {
    codecs : VideoCodecs[];
}
