/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

export type FocusResourceType = 'AudioContext';

/**
 * Top-level resource object for focus
 */
export interface IFocusResource {
  type : FocusResourceType;
}

/**
 * Audio focus resource
 */
export interface IAudioFocusResource extends IFocusResource {
  audioContext : AudioContext;
}
