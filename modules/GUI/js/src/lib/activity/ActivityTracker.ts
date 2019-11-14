/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

import { ActivityEvent } from './ActivityEvent';

/**
 * GUI activity tracker. Tracks components that is currently active and sends corresponding events.
 * See https://wiki.labcollab.net/confluence/display/Doppler/APL+Renderer+Idle+Handling
 * @export
 * @class
 */
export class ActivityTracker {
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
        this.currentActivityToken++;
        this.activeComponents.set(this.currentActivityToken, name);
        if (stateChanged) {
            this.reportActive();
        }
        return this.currentActivityToken;
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
