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
