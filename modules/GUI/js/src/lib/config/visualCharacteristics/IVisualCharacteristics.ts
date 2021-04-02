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
