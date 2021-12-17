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

import { ActivityEvent } from './ActivityEvent';
import { IActivityTracker } from './IActivityTracker';

/**
 * GUI activity tracker. Tracks components that is currently active and sends corresponding events.
 * See https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/SDK-GUI-API.md#activityevent
 * @export
 * @class
 */
export class ActivityTracker implements IActivityTracker {
    /// Interface to send out interaction state changed events.
    private activityEventCallback : (event : ActivityEvent) => void;

    /// Set of currently active components.
    private activeComponents : Map<number, string> = new Map();

    /// Activity token generator.
    private currentActivityToken = 0;

    constructor(activityEventCallback : (event : ActivityEvent) => void) {
        this.activityEventCallback = activityEventCallback;
    }

    /**
     * Record that an activity was interrupted
     */
    public reportInterrupted() {
        this.activityEventCallback(ActivityEvent.INTERRUPT);
    }

    /**
     * Record component as currently active.
     *
     * @param name Name of the component.
     * @returns ActivityTracker token
     */
    public recordActive(name : string) : number {
        const stateChanged = this.componentsEmpty();
        let prevToken : number = 0;
        this.activeComponents.forEach((value : string, key : number) => {
            if (value === name) {
                prevToken = key;
            }
        });
        if (prevToken === 0) {
            this.currentActivityToken++;
            this.activeComponents.set(this.currentActivityToken, name);
            if (stateChanged) {
                this.reportActive();
            }
            return this.currentActivityToken;
        } else {
            return prevToken;
        }
    }

    /**
     * Record component as currently inactive.
     *
     * @param token Tracking token.
     */
    public recordInactive(token : number) {
        this.activeComponents.delete(token);

        if (this.componentsEmpty()) {
            this.reportInactive();
        }
    }

    /**
     * Reset ActivityTracker state. Clear active components list and report as INACTIVE.
     */
    public reset() {
        this.activeComponents.clear();
        this.reportInactive();
    }

    private reportActive() {
        this.activityEventCallback(ActivityEvent.ACTIVATED);
    }

    private reportInactive() {
        this.activityEventCallback(ActivityEvent.DEACTIVATED);
    }

    private componentsEmpty() : boolean {
        return (this.activeComponents.size === 0);
    }
}
