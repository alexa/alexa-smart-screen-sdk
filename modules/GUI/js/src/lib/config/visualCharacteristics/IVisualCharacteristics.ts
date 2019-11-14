/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

import { IDeviceDisplay } from './IDeviceDisplay';
import { IDisplayWindowTemplate } from './IDisplayWindow';
import { IInteractionMode } from './IInteractionMode';
import { IVideo } from './IVideo';

/**
 * Wrapper interface for DCF reported characteristics
 */
export interface IVisualCharacteristics {
    /** Physical Device Display Characteristics - reported */
    display : IDeviceDisplay;
    /** Device Window Configurations - reported */
    windowTemplates : IDisplayWindowTemplate[];
    /** Device Interaction Modes - reported */
    interactionModes : IInteractionMode[];
    /** Device supported Video Codecs - reported */
    video : IVideo;
}
