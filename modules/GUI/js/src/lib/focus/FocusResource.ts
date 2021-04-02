/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
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
