/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */

'use strict';

/**
 * Copy of alexaSmartScreenSDK/smartScreenCapabilityAgents/renderingHandler/ActivityEvent
 *
 * @enum
 * @exports
 */
export enum ActivityEvent {
    /**
     * GUI switched to active state.
     */
    ACTIVATED = 'ACTIVATED',
    /**
     * GUI become inactive.
     */
    DEACTIVATED = 'DEACTIVATED',
    /**
     * GUI processed one-time event.
     */
    ONE_TIME = 'ONE_TIME',
    /**
     * Interrupt event (touch)
     */
    INTERRUPT = 'INTERRUPT',
    /**
     * Guard option for unknown event.
     */
    UNKNOWN = 'UNKNOWN'
}
