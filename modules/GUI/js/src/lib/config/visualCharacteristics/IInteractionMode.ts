/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { DeviceMode } from 'apl-client';

export type InteractionDistanceUnit =
    'CENTIMETER'
    | 'INCHES';

export type InteractionSupportType =
    'SUPPORTED'
    | 'UNSUPPORTED';

export interface IInteractionDistance {
    unit : InteractionDistanceUnit;
    value : number;
}

export interface IInteractionMode {
    id : string;
    uiMode : DeviceMode;
    interactionDistance : IInteractionDistance;
    touch : InteractionSupportType;
    keyboard : InteractionSupportType;
    video : InteractionSupportType;
}
